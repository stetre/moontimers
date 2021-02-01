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

#include "internal.h"
#define _ISOC99_SOURCE_
#include <math.h> /* for HUGE_VAL */

/* Tree of armed timers. Search key = timer->exptime */
static int cmp(timer_t *timer1, timer_t *timer2) 
    { return timer1->exptime < timer2->exptime ? -1 : 1; }

static RB_HEAD(ArmedTree, timer_s) ArmedHead = RB_INITIALIZER(&ArmedHead);
RB_PROTOTYPE_STATIC(ArmedTree, timer_s, ArmedEntry, cmp) 
RB_GENERATE_STATIC(ArmedTree, timer_s, ArmedEntry, cmp) 

static timer_t *ArmedRemove(timer_t *timer) { return RB_REMOVE(ArmedTree, &ArmedHead, timer); }
static timer_t *ArmedInsert(timer_t *timer) { return RB_INSERT(ArmedTree, &ArmedHead, timer); }
#if 0
static timer_t *ArmedSearch(uint32_t tid, double exptime)
    { timer_t tmp; tmp.exptime = exptime; tmp.tid = tid; return RB_FIND(ArmedTree, &ArmedHead, &tmp); }
#endif
static timer_t *ArmedFirst(void)
    { timer_t tmp; tmp.exptime = 0.0; return RB_NFIND(ArmedTree, &ArmedHead, &tmp); }
#if 0
static timer_t *ArmedNext(timer_t *timer) { return RB_NEXT(ArmedTree, &ArmedHead, timer); }
static timer_t *ArmedPrev(timer_t *timer) { return RB_PREV(ArmedTree, &ArmedHead, timer); }
static timer_t *ArmedMin(void) { return RB_MIN(ArmedTree, &ArmedHead); }
static timer_t *ArmedMax(void) { return RB_MAX(ArmedTree, &ArmedHead); }
static timer_t *ArmedRoot(void) { return RB_ROOT(&ArmedHead); }
#endif

static double Tnext = HUGE_VAL; // next time that a trigger() call is expected 

static int freetimer(lua_State *L, ud_t *ud)
    {
    timer_t *timer = (timer_t*)ud->handle;
//    freechildren(L, _HANDLER_MT, ud);
    if(!freeuserdata(L, ud, "timer")) return 0;
    if(IsArmed(ud)) ArmedRemove(timer);
    Free(L, timer);
    return 0;
    }

static int Create(lua_State *L)
    {
    ud_t *ud;
    timer_t *timer;
    double dur = luaL_checknumber(L, 1);
    if(!lua_isfunction(L, 2))
        return argerror(L, 2, ERR_FUNCTION);
    timer = Malloc(L, sizeof(timer_t));
    ud = newuserdata(L, timer, TIMER_MT, "timer");
    Reference(L, 2, ud->ref1);
    timer->duration = dur;
    timer->exptime = 0.0f;
    ud->parent_ud = NULL;
    ud->destructor = freetimer;
    ud->info = NULL;
    return 1;
    }

static int Start(lua_State *L)
    {
    ud_t *ud;
    timer_t *timer = checktimer(L, 1, &ud);
    double exptime = luaL_optnumber(L, 2, now()+timer->duration);
    if(IsArmed(ud))
        { ArmedRemove(timer); CancelArmed(ud); }
    timer->exptime = exptime;
    ArmedInsert(timer); MarkArmed(ud);
    Tnext = exptime < Tnext ? exptime : Tnext;
    lua_pushnumber(L, exptime);
    return 1;
    }

static int Stop(lua_State *L)
    {
    ud_t *ud;
    timer_t *timer = checktimer(L, 1, &ud);
    if(IsArmed(ud))
        { ArmedRemove(timer); CancelArmed(ud); }
    return 0;
    }

static int Set_duration(lua_State *L)
    {
    ud_t *ud;
    timer_t *timer = checktimer(L, 1, &ud);
    timer->duration = luaL_checknumber(L, 2);
    return 0;
    }

static int Set_callback(lua_State *L)
    {
    ud_t *ud;
    (void)checktimer(L, 1, &ud);
    if(!lua_isfunction(L, 2))
        return argerror(L, 2, ERR_FUNCTION);
    Reference(L, 2, ud->ref1);
    return 0;
    }

static int Is_running(lua_State *L)
    {
    ud_t *ud;
    timer_t *timer = checktimer(L, 1, &ud);
    if(IsArmed(ud))
        {
        lua_pushboolean(L, 1);
        lua_pushnumber(L, timer->exptime);
        }
    else
        {
        lua_pushboolean(L, 0);
        lua_pushnumber(L, HUGE_VAL);
        }
    return 2;
    }

static int Trigger(lua_State *L)
/* Execute the callbacks for all timers with exptime <= now */
    {
    int top;
    timer_t *timer;
    ud_t *ud;
    double tnow = now();
    if(tnow < Tnext) goto done;
    while((timer = ArmedFirst())!=NULL)
        {
        if(timer->exptime > tnow) break;
        top = lua_gettop(L);
        // timer expired, execute callback
        ud = userdata(timer);
        ArmedRemove(timer); CancelArmed(ud);
        if(lua_rawgeti(L, LUA_REGISTRYINDEX, ud->ref1) != LUA_TFUNCTION)
            return unexpected(L);
        pushtimer(L, timer);
        lua_pushnumber(L, timer->exptime);
        if(lua_pcall(L, 2, 0, 0) != LUA_OK)
            return lua_error(L);
        lua_settop(L, top);
        }
    timer = ArmedFirst();
    Tnext = timer ? timer->exptime : HUGE_VAL;
done:
    lua_pushnumber(L, Tnext);
    return 1;
    }

static int T_next(lua_State *L)
    {
    lua_pushnumber(L, Tnext);
    return 1;
    }

DESTROY_FUNC(timer)

static const struct luaL_Reg Methods[] = 
    {
        { "free", Destroy },
        { "start", Start },
        { "stop", Stop },
        { "set_duration", Set_duration },
        { "set_callback", Set_callback },
        { "is_running", Is_running },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg MetaMethods[] = 
    {
        { "__gc",  Destroy },
        { NULL, NULL } /* sentinel */
    };

static const struct luaL_Reg Functions[] = 
    {
        { "new", Create },
        { "trigger", Trigger },
        { "tnext", T_next },
        { NULL, NULL } /* sentinel */
    };

void moontimers_open_timer(lua_State *L)
    {
    udata_define(L, TIMER_MT, Methods, MetaMethods);
    luaL_setfuncs(L, Functions, 0);
    }

