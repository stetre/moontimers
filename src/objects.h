/* The MIT License (MIT)
 *
 * Copyright (c) 2021 Stefano Trettel
 *
 * Software repository: MoonTimers, https://github.com/stetre/moontimers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef objectsDEFINED
#define objectsDEFINED

#include "tree.h"
#include "udata.h"

#define timer_t struct timer_s
struct timer_s {
    RB_ENTRY(timer_s) ArmedEntry;
    double duration;  /* default duration (seconds) */
    double exptime;   /* expiration time based on now() */
    uint16_t status;  /* timer status */
};

/* Objects' metatable names */
#define TIMER_MT "moontimers_timer"

/* Userdata memory associated with objects */
#define ud_t moontimers_ud_t
typedef struct moontimers_ud_s ud_t;

struct moontimers_ud_s {
    void *handle; /* the object handle bound to this userdata */
    int (*destructor)(lua_State *L, ud_t *ud);  /* self destructor */
    ud_t *parent_ud; /* the ud of the parent object */
    uint32_t marks;
    int ref1, ref2, ref3, ref4; /* refs for callbacks, automatically unreferenced at destruction */
    void *info; /* object specific info (ud_info_t, subject to Free() at destruction, if not NULL) */
};
    
/* Marks.  m_ = marks word (uint32_t) , i_ = bit number (0 .. 31)  */
#define MarkGet(m_,i_)  (((m_) & ((uint32_t)1<<(i_))) == ((uint32_t)1<<(i_)))
#define MarkSet(m_,i_)  do { (m_) = ((m_) | ((uint32_t)1<<(i_))); } while(0)
#define MarkReset(m_,i_) do { (m_) = ((m_) & (~((uint32_t)1<<(i_)))); } while(0)

#define IsValid(ud)             MarkGet((ud)->marks, 0)
#define MarkValid(ud)           MarkSet((ud)->marks, 0) 
#define CancelValid(ud)         MarkReset((ud)->marks, 0)

#define IsArmed(ud)             MarkGet((ud)->marks, 1)
#define MarkArmed(ud)           MarkSet((ud)->marks, 1) 
#define CancelArmed(ud)         MarkReset((ud)->marks, 1)

#if 0
/* .c */
#define  moontimers_
#endif

#define setmetatable moontimers_setmetatable
int setmetatable(lua_State *L, const char *mt);

#define newuserdata moontimers_newuserdata
ud_t *newuserdata(lua_State *L, void *handle, const char *mt, const char *tracename);
#define freeuserdata moontimers_freeuserdata
int freeuserdata(lua_State *L, ud_t *ud, const char *tracename);
#define pushuserdata moontimers_pushuserdata 
int pushuserdata(lua_State *L, ud_t *ud);

#define freechildren moontimers_freechildren
int freechildren(lua_State *L,  const char *mt, ud_t *parent_ud);

#define userdata_unref(L, handle) udata_unref((L),(handle))

#define UD(handle) userdata((handle)) /* dispatchable objects only */
#define userdata moontimers_userdata
ud_t *userdata(const void *handle);
#define testxxx moontimers_testxxx
void *testxxx(lua_State *L, int arg, ud_t **udp, const char *mt);
#define checkxxx moontimers_checkxxx
void *checkxxx(lua_State *L, int arg, ud_t **udp, const char *mt);
#define optxxx moontimers_optxxx
void *optxxx(lua_State *L, int arg, ud_t **udp, const char *mt);
#define pushxxx moontimers_pushxxx
int pushxxx(lua_State *L, void *handle);
#define checkxxxlist moontimers_checkxxxlist
void** checkxxxlist(lua_State *L, int arg, int *count, int *err, const char *mt);

/* timer.c */
#define checktimer(L, arg, udp) (timer_t*)checkxxx((L), (arg), (udp), TIMER_MT)
#define testtimer(L, arg, udp) (timer_t*)testxxx((L), (arg), (udp), TIMER_MT)
#define opttimer(L, arg, udp) (timer_t*)optxxx((L), (arg), (udp), TIMER_MT)
#define pushtimer(L, handle) pushxxx((L), (void*)(handle))
#define checktimerlist(L, arg, count, err) checkxxxlist((L), (arg), (count), (err), TIMER_MT)

#if 0 // 7yy
/* zzz.c */
#define checkzzz(L, arg, udp) (zzz_t*)checkxxx((L), (arg), (udp), ZZZ_MT)
#define testzzz(L, arg, udp) (zzz_t*)testxxx((L), (arg), (udp), ZZZ_MT)
#define optzzz(L, arg, udp) (zzz_t*)optxxx((L), (arg), (udp), ZZZ_MT)
#define pushzzz(L, handle) pushxxx((L), (void*)(handle))
#define checkzzzlist(L, arg, count, err) checkxxxlist((L), (arg), (count), (err), ZZZ_MT)

#endif

#define RAW_FUNC(xxx)                       \
static int Raw(lua_State *L)                \
    {                                       \
    lua_pushinteger(L, (uintptr_t)check##xxx(L, 1, NULL));  \
    return 1;                               \
    }

#define TYPE_FUNC(xxx) /* NONCL */          \
static int Type(lua_State *L)               \
    {                                       \
    (void)check##xxx(L, 1, NULL);           \
    lua_pushstring(L, ""#xxx);              \
    return 1;                               \
    }

#define DESTROY_FUNC(xxx)                   \
static int Destroy(lua_State *L)            \
    {                                       \
    ud_t *ud;                               \
    (void)test##xxx(L, 1, &ud);             \
    if(!ud) return 0; /* already deleted */ \
    return ud->destructor(L, ud);           \
    }

#define PARENT_FUNC(xxx)                    \
static int Parent(lua_State *L)             \
    {                                       \
    ud_t *ud;                               \
    (void)check##xxx(L, 1, &ud);            \
    if(!ud->parent_ud) return 0;            \
    return pushuserdata(L, ud->parent_ud);  \
    }

#endif /* objectsDEFINED */
