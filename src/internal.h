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

#ifndef internalDEFINED
#define internalDEFINED

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "moontimers.h"

#define TOSTR_(x) #x
#define TOSTR(x) TOSTR_(x)

#include "tree.h"
#include "objects.h"

/* Note: all the dynamic symbols of this library (should) start with 'moontimers_' .
 * The only exception is the luaopen_moontimers() function, which is searched for
 * with that name by Lua.
 * MoonTimers's string references on the Lua registry also start with 'moontimers_'.
 */

#if 0
/* .c */
#define  moontimers_
#endif

/* flags.c */
#define checkflags(L, arg) luaL_checkinteger((L), (arg))
#define optflags(L, arg, defval) luaL_optinteger((L), (arg), (defval))
#define pushflags(L, val) lua_pushinteger((L), (val))

/* utils.c */
void moontimers_utils_init(lua_State *L);
#define noprintf moontimers_noprintf
int noprintf(const char *fmt, ...); 
#define now moontimers_now
double now(void);
#define sleeep moontimers_sleeep
void sleeep(double seconds);
#define since(t) (now() - (t))
#define notavailable moontimers_notavailable
int notavailable(lua_State *L, ...);
#define Malloc moontimers_Malloc
void *Malloc(lua_State *L, size_t size);
#define MallocNoErr moontimers_MallocNoErr
void *MallocNoErr(lua_State *L, size_t size);
#define Strdup moontimers_Strdup
char *Strdup(lua_State *L, const char *s);
#define Free moontimers_Free
void Free(lua_State *L, void *ptr);
#define checkboolean moontimers_checkboolean
int checkboolean(lua_State *L, int arg);
#define testboolean moontimers_testboolean
int testboolean(lua_State *L, int arg, int *err);
#define optboolean moontimers_optboolean
int optboolean(lua_State *L, int arg, int d);

/* Internal error codes */
#define ERR_NOTPRESENT       1
#define ERR_SUCCESS          0
#define ERR_GENERIC         -1
#define ERR_TYPE            -2
#define ERR_ELEMTYPE        -3
#define ERR_VALUE           -4
#define ERR_ELEMVALUE       -5
#define ERR_TABLE           -6
#define ERR_FUNCTION        -7
#define ERR_EMPTY           -8
#define ERR_MEMORY          -9
#define ERR_MALLOC_ZERO     -10
#define ERR_LENGTH          -11
#define ERR_POOL            -12
#define ERR_BOUNDARIES      -13
#define ERR_RANGE           -14
#define ERR_FOPEN           -15
#define ERR_OPERATION       -16
#define ERR_UNKNOWN         -17
#define errstring moontimers_errstring
const char* errstring(int err);

/* tracing.c */
#define trace_objects moontimers_trace_objects
extern int trace_objects;

/* main.c */
extern lua_State *moontimers_L;
int luaopen_moontimers(lua_State *L);
void moontimers_open_tracing(lua_State *L);
void moontimers_open_timer(lua_State *L);

/*------------------------------------------------------------------------------*
 | Debug and other utilities                                                    |
 *------------------------------------------------------------------------------*/

/* If this is printed, it denotes a suspect bug: */
#define UNEXPECTED_ERROR "unexpected error (%s, %d)", __FILE__, __LINE__
#define unexpected(L) luaL_error((L), UNEXPECTED_ERROR)

/* Errors with internal error code (ERR_XXX) */
#define failure(L, errcode) luaL_error((L), errstring((errcode)))
#define argerror(L, arg, errcode) luaL_argerror((L), (arg), errstring((errcode)))
#define errmemory(L) luaL_error((L), errstring((ERR_MEMORY)))

#define notsupported(L) luaL_error((L), "operation not supported")
#define badvalue(L, s)   lua_pushfstring((L), "invalid value '%s'", (s))

/* Reference/unreference variables on the Lua registry */
#define Unreference(L, ref) do {                        \
    if((ref)!= LUA_NOREF)                               \
        {                                               \
        luaL_unref((L), LUA_REGISTRYINDEX, (ref));      \
        (ref) = LUA_NOREF;                              \
        }                                               \
} while(0)

#define Reference(L, arg, ref)  do {                    \
    Unreference((L), (ref));                            \
    lua_pushvalue(L, (arg));                            \
    (ref) = luaL_ref(L, LUA_REGISTRYINDEX);             \
} while(0)

/* DEBUG -------------------------------------------------------- */
#if defined(DEBUG)

#define DBG printf
#define TR() do { printf("trace %s %d\n",__FILE__,__LINE__); } while(0)
#define BK() do { printf("break %s %d\n",__FILE__,__LINE__); getchar(); } while(0)
#define TSTART double ts = now();
#define TSTOP do {                                          \
    ts = since(ts); ts = ts*1e6;                            \
    printf("%s %d %.3f us\n", __FILE__, __LINE__, ts);      \
    ts = now();                                             \
} while(0);

#else 

#define DBG noprintf
#define TR()
#define BK()
#define TSTART do {} while(0) 
#define TSTOP do {} while(0)    

#endif /* DEBUG ------------------------------------------------- */

#endif /* internalDEFINED */
