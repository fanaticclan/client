/*
------------------------------------------------------------------------------
Copyright (c) 2013 cm|ac

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
------------------------------------------------------------------------------
            Modified for Fanatic Edition 2014 by Nyne and bum
*/

#include "engine.h"
#include "game.h"
#include "lua.hpp"

lua_State *L;

void lua_error(lua_State *L, int status)
{
    if(status !=0)
    {
        conoutf("\f9FanEd\f7::LuaJIT: \f3#error: \f7%s", lua_tostring(L, -1));
        playsound(S_ERROR);
        lua_pop(L, 1);
    }
}
ICOMMAND(lua, "s", (char *s), { int k = luaL_dostring(L, s); lua_error(L, k); }); 

void execlua(const char *luafile)
{
    int status = luaL_loadfile(L, luafile);
    if(status == 0){ status = lua_pcall(L, 0, LUA_MULTRET, 0); }
    lua_error(L, status);
}
ICOMMAND(execlua, "s", (char *file), execlua(file));

int lua_sauer(lua_State *L)
{
    int argc = lua_gettop(L);
    int n;
    char *ret;
    
    for (n = 1; n <= argc; n++)
    {
        ret = executestr(lua_tostring(L, n));
        lua_pushstring(L, ret);
    }
    
    return argc;
}

int lua_build(lua_State *L)
{
    if(lua_gettop(L) < 13)
    {
        conoutf("\f9FanEd\f7::LuaJIT: \f2#help: \f7sauer_build x y z grid face face face tex tex tex tex tex tex");
        conoutf("\f9FanEd\f7::LuaJIT: \f2#help: \f7full face = 80808080; coordinates range = 0 to 2^mapsize; grid = 1 to mapsize");
    }
    int x = lua_tonumber(L, 1);
    int y = lua_tonumber(L, 2);
    int z = lua_tonumber(L, 3);
    int grid = lua_tonumber(L, 4);
    
    uint face[3];
    loopi(3) face[i] = (uint)(lua_tonumber(L, i + 5));
    
    int tex[6];
    loopi(6) tex[i] = (int)(lua_tonumber(L, i + 8));
    
    cube *c;
    c = &lookupcube(x, y, z, grid);
    if(c->children) discardchildren(*c, true);
    loopi(3) c->faces[i] = face[i];
    loopi(6) c->texture[i] = tex[i];
    return 0;
}

int lua_copy(lua_State *L)
{
    if(lua_gettop(L)<13) conoutf("\f9FanEd\f7::LuaJIT: \f2#help: \f7sauer_copy selox seloy seloz selsx selsy selsz selgrid selcx selcy selcys selorient selcorner");

    selinfo cmsel;
    cmsel.o.x = lua_tonumber(L, 1);
    cmsel.o.y = lua_tonumber(L, 2);
    cmsel.o.z= lua_tonumber(L, 3);
    cmsel.s.x = lua_tonumber(L, 4);
    cmsel.s.y = lua_tonumber(L, 5);
    cmsel.s.z = lua_tonumber(L, 6);
    cmsel.grid = lua_tonumber(L, 7);
    cmsel.cx = lua_tonumber(L, 8);
    cmsel.cxs = lua_tonumber(L, 9);
    cmsel.cy = lua_tonumber(L, 10);
    cmsel.cys = lua_tonumber(L, 11);
    cmsel.orient = lua_tonumber(L, 12);
    cmsel.corner = lua_tonumber(L, 13);

    mpcopy(localedit, cmsel, true);
    return 0;
}

int lua_paste(lua_State *L)
{
    if(lua_gettop(L)<13) conoutf("\f9FanEd\f7::LuaJIT: \f2#help: \f7sauer_paste selox seloy seloz selsx selsy selsz selgrid selcx selcy selcys selorient selcorner");

    selinfo newsel;
    newsel.o.x = lua_tonumber(L, 1);
    newsel.o.y = lua_tonumber(L, 2);
    newsel.o.z= lua_tonumber(L, 3);
    newsel.s.x = lua_tonumber(L, 4);
    newsel.s.y = lua_tonumber(L, 5);
    newsel.s.z = lua_tonumber(L, 6);
    newsel.grid = lua_tonumber(L, 7);
    newsel.cx = lua_tonumber(L, 8);
    newsel.cxs = lua_tonumber(L, 9);
    newsel.cy = lua_tonumber(L, 10);
    newsel.cys = lua_tonumber(L, 11);
    newsel.orient = lua_tonumber(L, 12);
    newsel.corner = lua_tonumber(L, 13);

    mppaste(localedit, newsel, true);
    return 0;
}

int lua_editface(lua_State *L)
{
    if(lua_gettop(L)<15) conoutf("\f9FanEd\f7::LuaJIT: \f2#help: \f7sauer_editface selox seloy seloz selsx selsy selsz selgrid selcx selcxs selcy selcys selorient selcorner dir mode");

    selinfo newsel;
    newsel.o.x = lua_tonumber(L, 1);
    newsel.o.y = lua_tonumber(L, 2);
    newsel.o.z= lua_tonumber(L, 3);
    newsel.s.x = lua_tonumber(L, 4);
    newsel.s.y = lua_tonumber(L, 5);
    newsel.s.z = lua_tonumber(L, 6);
    newsel.grid = lua_tonumber(L, 7);
    newsel.cx = lua_tonumber(L, 8);
    newsel.cxs = lua_tonumber(L, 9);
    newsel.cy = lua_tonumber(L, 10);
    newsel.cys = lua_tonumber(L, 11);
    newsel.orient = lua_tonumber(L, 12);
    newsel.corner = lua_tonumber(L, 13);
    int dir = lua_tonumber(L, 14);
    int mode = lua_tonumber(L, 15);

    mpeditface(dir, mode, newsel, true);
    return 0;
}

int lua_edittex(lua_State *L)
{
    if(lua_gettop(L)<15)
    {
        conoutf("\f9FanEd\f7::LuaJIT: \f2#help: \f7changes tex at specified position, if cube doesn't exist it's created");
        conoutf("\f9FanEd\f7::LuaJIT: \f2#help: \f7sauer_edittex selox seloy seloz selsx selsy selsz selgrid selcx selcy selcys selorient selcorner tex allfaces");
    }

    selinfo newsel;
    newsel.o.x = lua_tonumber(L, 1);
    newsel.o.y = lua_tonumber(L, 2);
    newsel.o.z= lua_tonumber(L, 3);
    newsel.s.x = lua_tonumber(L, 4);
    newsel.s.y = lua_tonumber(L, 5);
    newsel.s.z = lua_tonumber(L, 6);
    newsel.grid = lua_tonumber(L, 7);
    newsel.cx = lua_tonumber(L, 8);
    newsel.cxs = lua_tonumber(L, 9);
    newsel.cy = lua_tonumber(L, 10);
    newsel.cys = lua_tonumber(L, 11);
    newsel.orient = lua_tonumber(L, 12);
    newsel.corner = lua_tonumber(L, 13);
    int tex = lua_tonumber(L, 14);
    int allfaces = lua_tonumber(L, 15);

    mpedittex(tex, allfaces, newsel, true);
    return 0;
}

int lua_editent(lua_State *L)
{
    int args = lua_gettop(L);
    if(args < 10)
    {
        conoutf("\f9FanEd\f7::LuaJIT: \f2#help: \f7sauer_editent index x y z type attr1 attr2 attr3 attr4 attr5 [local]");
        return 0;
    }
    
    int i = lua_tonumber(L, 1);
    int x = lua_tonumber(L, 2);
    int y = lua_tonumber(L, 3);
    int z = lua_tonumber(L, 4);
    int type = lua_tonumber(L, 5);
    int attr[5];
    loopi(5) attr[i] = (int)(lua_tonumber(L, i + 6));
    bool local = true;
    if(args > 10) local = lua_toboolean(L, 11);
    
    mpeditent(i, vec(x, y, z), type, attr[0], attr[1], attr[2], attr[3], attr[4], local);
    return 0;
}

extern int isemptycube(int tx, int ty, int tz, int tscale);

int lua_getcubeinfo(lua_State *L)
{
    int argc = lua_gettop(L);

    if(argc==0) return 0;

    if(lua_tonumber(L, 1) == -1)
    {
        lua_pushnumber(L, isemptycube(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5)));
        return 1;
    }

    fpsent *t = game::getclient(lua_tonumber(L, 1));
    if(t == 0) return 0;

    vec unitv(lua_tonumber(L, 2),lua_tonumber(L, 3),lua_tonumber(L, 4));
    int type = lua_tonumber(L, 6);
    float barrier;

    if(unitv.x == 0 && unitv.y == 0 && unitv.z == 0) barrier = 0.;
    else
    {
        unitv.normalize();
        float dist = lua_tonumber(L, 5);
        barrier = raycube(t->o, unitv, dist, RAY_CLIPMAT|RAY_ALPHAPOLY);
        if (type==-2) {lua_pushnumber(L,barrier); return 1;}
    }

    cube &c = lookupcube(int(t->o.x+barrier*unitv.x), int(t->o.y+barrier*unitv.y), int(t->o.z+barrier*unitv.z), -1);

    switch (type)
    {
        case -1: lua_pushnumber(L, c.material); break;
        case  0:
        case  1:
        case  2:
        case  3:
        case  4:
        case  5: lua_pushnumber(L, c.texture[type]); break;
        default: return 0;
    }
    return 1;
}

int lua_getsel(lua_State *L)
{
    int args = lua_gettop(L);
    if(args < 1 ) {
        conoutf("\f9FanEd\f7::LuaJIT: \f2#help: \f7sauer_getsel type1 type2 ... typen");
        conoutf("\f9FanEd\f7::LuaJIT: \f2#help: \f7types: 0 o.x; 1 o.y; 2 o.z; 3 s.x; 4 s.y; 5 s.z; 6 grid; .. 11 orient; 12: sel.corner");
        return 0;
    }
    
    int ret = 0, i, type;
    extern selinfo sel;
    for (i = 1; i <= args; i++)
    {
        type = lua_tonumber(L, i);
        bool invalid = false;
        switch (type)
        {
            case 0:  ret = sel.o.x;     break;
            case 1:  ret = sel.o.y;     break;
            case 2:  ret = sel.o.z;     break;
            case 3:  ret = sel.s.x;     break;
            case 4:  ret = sel.s.y;     break;
            case 5:  ret = sel.s.z;     break;
            case 6:  ret = sel.grid;    break;
            case 7:  ret = sel.cx;      break;
            case 8:  ret = sel.cxs;     break;
            case 9:  ret = sel.cy;      break;
            case 10: ret = sel.cys;     break;
            case 11: ret = sel.orient;  break;
            case 12: ret = sel.corner;  break;
            default: invalid = true;    break;
        }
        if(!invalid) lua_pushnumber(L, ret);
        else lua_pushnil(L);
    }
    
    return args;
}   

int lua_setsel(lua_State *L)
{
    int args = lua_gettop(L);
    if(args < 7)
    {
        conoutf("\f9FanEd\f7::LuaJIT: \f2#help: \f7sauer_setsel x y z x2 y2 z2 gridpower orientation");
        return 0;
    }
    
    extern selinfo sel;
    extern bool havesel;
    extern int gridpower, gridsize;
    
    int ox = lua_tonumber(L, 1);
    int oy = lua_tonumber(L, 2);
    int oz = lua_tonumber(L, 3);
    int sx = lua_tonumber(L, 4);
    int sy = lua_tonumber(L, 5);
    int sz = lua_tonumber(L, 6);
    int gp = lua_tonumber(L, 7);
    int orientation = 0;
    if(args > 7)
        orientation = lua_tonumber(L, 8);
    
    gridpower = gp;
    gridsize = 1 << gridpower;
    
    sel.o.x = ox * gridsize;
    sel.o.y = oy * gridsize;
    sel.o.z = oz * gridsize;
    sel.s.x = sx;
    sel.s.y = sy;
    sel.s.z = sz;
    sel.orient = orientation;
    sel.grid = gridsize;
    
    havesel = true;
    return 0;
}

extern void mpeditvslot(VSlot &ds, int allfaces, selinfo& sel, bool local);

int lua_vcolor(lua_State *L)
{
    int args = lua_gettop(L);
    if(args < 11)
    {
        conoutf("\f9FanEd\f7::LuaJIT: \f2#help: \f7sauer_vcolor ox oy oz sx sy sz grid r g b face");
        return 0;
    }
    
    selinfo newsel;
    newsel.o.x = lua_tonumber(L, 1);
    newsel.o.y = lua_tonumber(L, 2);
    newsel.o.z = lua_tonumber(L, 3);
    newsel.s.x = lua_tonumber(L, 4);
    newsel.s.y = lua_tonumber(L, 5);
    newsel.s.z = lua_tonumber(L, 6);
    newsel.grid = lua_tonumber(L, 7);
    
    float r = lua_tonumber(L, 8);
    float g = lua_tonumber(L, 9);
    float b = lua_tonumber(L, 10);
    
    newsel.orient = lua_tonumber(L, 11);
    int allfaces = 0;
    if(newsel.orient < 0 || newsel.orient > 5)
        allfaces = 1;
    
    VSlot ds;
    ds.changed = 1 << VSLOT_COLOR;
    ds.colorscale = vec(clamp(r, 0.0f, 1.0f), clamp(g, 0.0f, 1.0f), clamp(b, 0.0f, 1.0f));
    mpeditvslot(ds, allfaces, newsel, true);
    return 0;
}

extern bool hashstring(const char *str, char *result, int maxlen);

int lua_hashstring(lua_State *L)
{
    int args = lua_gettop(L);
    if(args < 1)
    {
        conoutf("\f9FanEd\f7::LuaJIT: \f2#help: \f7sauer_hashstring message [maxlen]");
        return 0;
    }
    
    if(!lua_isstring(L, 1))
    {
        conoutf("\f9FanEd\f7::LuaJIT: \f3#error: \f7sauer_hashstring message (arg1) must be a string!");
        playsound(S_ERROR);
        return 0;
    }
    
    const char *str = (char*) lua_tostring(L, 1);
    int maxlen = 49;
    if(args > 1)
    {
        int tmp = lua_tonumber(L, 2);
        if(tmp > 49)
        {
            maxlen = tmp;
            conoutf("\f9FanEd\f7::LuaJIT: \f6#warning: \f7sauer_hashstring maxlen (arg2) must be greater than 49!");
        }
    }
    char *result = new char[maxlen];
    
    hashstring(str, result, maxlen);
    lua_pushstring(L, result);
    return 1;
}

extern SDL_Surface *loadsurface(const char *name);

int lua_loadimage(lua_State *L)
{
    int args = lua_gettop(L);
    if(args < 1)
    {
        conoutf("\f9FanEd\f7::LuaJIT: \f2#help: \f7sauer_loadimage file [width height]");
        return 0;
    }
    if(!lua_isstring(L, 1))
    {
        conoutf("\f9FanEd\f7::LuaJIT: \f3#error: \f7sauer_loadimage file (arg1) not a string!");
        playsound(S_ERROR);
        return 0;
    }
    const char *filename = lua_tostring(L, 1);
    ImageData image;
    SDL_Surface *s = loadsurface(path(filename, true));
    if(!s) return 0;
    
    int width = image.w, height = image.h;
    if(args > 1) width = lua_tonumber(L, 2);
    if(args > 2) height = lua_tonumber(L, 3);
    
    image.wrap(s);
    
    int *red = new int[image.w * image.h];
    int *green = new int[image.w * image.h];
    int *blue = new int[image.w * image.h];
    
    ImageData d(image.w, image.h, 3);
    uchar *dstrow = d.data, *srcrow = image.data;
    int k = 0;
    loop(y, d.h)
    {
        for(uchar *dst = dstrow, *src = srcrow, *end = &srcrow[image.w * image.bpp]; src < end; dst += d.bpp, src += image.bpp)
        {
            red[k] = src[0];
            green[k] = src[1];
            blue[k] = src[2];
            k++;
        }
        dstrow += d.pitch;
        srcrow += image.pitch;
    }
    
    lua_createtable(L, width * height, 0);
    int index = 0, luaindex = 0;
    loopi(width)
    {
        loopj(height)
        {
            luaindex = j + (i * height);
            index = int((1.0 * i / width) * image.w) + int(1.0 * j / height * image.h) * image.w;
            lua_pushnumber(L, luaindex);
            lua_createtable(L, 3, 0);
            lua_pushnumber(L, 1);
            lua_pushnumber(L, red[index]);
            lua_settable(L, -3);
            lua_pushnumber(L, 2);
            lua_pushnumber(L, green[index]);
            lua_settable(L, -3);
            lua_pushnumber(L, 3);
            lua_pushnumber(L, blue[index]);
            lua_settable(L, -3);
            lua_settable(L, -3);
        }
    }
    delete red;
    delete green;
    delete blue;
    return 1;
}

int lua_lookupcube(lua_State *L)
{
    int argc = lua_gettop(L);
    if(argc < 4) return 0;
    cube *c = &lookupcube(int(lua_tonumber(L, 1)), int(lua_tonumber(L, 2)), int(lua_tonumber(L, 3)),-1);
    int i, type;
    for(i = 1; i <= argc-3; i++)
    {
        type = lua_tonumber(L, i+3);
        switch (type)
        {
            case -1: lua_pushnumber(L,(int)c); break;
            case  0:
            case  1:
            case  2: lua_pushnumber(L,c->faces[type]); break;
            case  3:
            case  4:
            case  5:
            case  6:
            case  7:
            case  8: lua_pushnumber(L,c->texture[type-3]); break;
            case  9: lua_pushnumber(L,c->material); break;
        }
    }
    return argc-3;
}

void lua_N_DAMAGE(int acn, int tcn, int damage, int armour, int health)
{
    lua_getglobal(L, "lua_N_DAMAGE"); lua_pushnumber(L, acn); lua_pushnumber(L, tcn); lua_pushnumber(L, damage);lua_pushnumber(L, armour); lua_pushnumber(L, health);
    int status = lua_pcall(L, 5, 0,0); if(status != LUA_OK) { lua_error(L, status); return; }
}

void lua_N_DIED(int vcn, int acn, int frags, int tfrags)
{
    lua_getglobal(L, "lua_N_DIED"); lua_pushnumber(L, vcn); lua_pushnumber(L, acn); lua_pushnumber(L, frags);lua_pushnumber(L, tfrags);
    int status = lua_pcall(L, 4, 0,0); if(status != LUA_OK) { lua_error(L, status); return; }
}

void lua_N_EDITMODE(int cn, int val)
{
    lua_getglobal(L, "lua_N_EDITMODE");	lua_pushnumber(L, cn); lua_pushnumber(L, val);
    int status = lua_pcall(L, 2, 0,0); if(status != LUA_OK) { lua_error(L, status); return; }
}

void lua_N_EVENT(int type, int arg1, int arg2, int arg3, int arg4)
{
    lua_getglobal(L, "lua_N_EVENT");
    lua_pushnumber(L, type);
    lua_pushnumber(L, arg1);
    lua_pushnumber(L, arg2);
    lua_pushnumber(L, arg3);
    lua_pushnumber(L, arg4);
    int status = lua_pcall(L, 5, 0,0); if(status != LUA_OK) { lua_error(L, status); return; }
}

void lua_N_SAYTEAM(int cn, char * text)
{
    lua_getglobal(L, "lua_N_SAYTEAM"); lua_pushnumber(L, cn); lua_pushstring(L, text);
    int status = lua_pcall(L, 2, 0,0); if(status != LUA_OK) { lua_error(L, status); return; }
}

void lua_N_SERVCMD(int cn, char * text)
{
    lua_getglobal(L, "lua_N_SERVCMD"); lua_pushnumber(L, cn); lua_pushstring(L, text);
    int status = lua_pcall(L, 2, 0,0); if(status != LUA_OK) { lua_error(L, status); return; }
}

void lua_N_TEXT(int cn, char * text)
{
    lua_getglobal(L, "lua_N_TEXT"); lua_pushnumber(L, cn); lua_pushstring(L, text);
    int status = lua_pcall(L, 2, 0,0); if(status != LUA_OK) { lua_error(L, status); return; }
}

void lua_N_SHOTFX(int cn, int gun, vec *to)
{
    lua_getglobal(L, "lua_N_SHOTFX");lua_pushnumber(L, cn); lua_pushnumber(L, gun); lua_pushnumber(L, to->x);lua_pushnumber(L, to->y); lua_pushnumber(L, to->z);
    int status = lua_pcall(L, 5, 0,0); if(status != LUA_OK) { lua_error(L, status); return; }
}

namespace game { extern fpsent *getclient(int cn); }
namespace game { extern fpsent *player1; }

int lua_getplayerinfo(lua_State *L)
{
    int argc = lua_gettop(L);
    int hcn = lua_tonumber(L, 1);
    int i;
    int type;
    float ret;

    fpsent *t = game::getclient(hcn);
    if(argc == 0) return 0;
    if(hcn < 0) return 0;
    if(t == 0) return 0;

    for (i = 2; i <= argc; i++)
    {
        type = lua_tonumber(L, i);
        switch (type)
        {
            case 1: ret=t->o.x; break;
            case 2: ret=t->o.y; break;
            case 3: ret=t->o.z; break;
            case 4: ret=t->pitch; break;
            case 5: ret=t->yaw; break;
            case 6: ret=t->vel.x; break;
            case 7: ret=t->vel.y; break;
            case 8: ret=t->vel.z ; break;
            case 9: ret = t->health; break;
            case 10: ret = t->armour; break;
            case 11: ret = t->armourtype; break;
            case 12: ret = t->gunselect ; break;
            case 13: ret = t->state; break;
            case 14: ret = t->vel.magnitude2(); break;
            case 15: ret = t->timeinair; break;
            case 18: ret = t->physstate; break;
            case 19: ret = t->ping; break;
            default: ret = -1;
        }
        if(type < 16 || type > 17) { lua_pushnumber(L, ret); }
        else if (type >= 16)
        {
            switch (type)
            {
                case 16: lua_pushstring(L, t->name); break;
                case 17: lua_pushstring(L, t->team); break;
            }
        }
    }
    return argc-1;
}

int lua_setplayerinfo(lua_State *L)
{
    int type = lua_tonumber(L, 1);
    float value = lua_tonumber(L, 2);

    switch (type)
    {
        case -1: game::doattack(value == 1); break;
        case 0: return 0;
        case 1: game::player1->o.x = value; game::player1->resetinterp(); break;
        case 2: game::player1->o.y = value; game::player1->resetinterp(); break;
        case 3: game::player1->o.z = value; game::player1->resetinterp(); break;
        case 4: game::player1->pitch = value; break;
        case 5: game::player1->yaw = value; break;
        case 6: game::player1->vel.x = value; break;
        case 7: game::player1->vel.y = value; break;
        case 8: game::player1->vel.z = value; break;
        case 9: game::player1->health = value; break;
        case 10: game::player1->move = value; break;
        case 11: game::player1->strafe = value; break;
        case 13: game::player1->state = value; break;
    }
    return 0;
}

int init_lua()
{
    L = luaL_newstate();
    luaL_openlibs(L);
    lua_register(L, "sauer",                lua_sauer);
    lua_register(L, "sauer_build",          lua_build);
    lua_register(L, "sauer_copy",           lua_copy);
    lua_register(L, "sauer_paste",          lua_paste);
    lua_register(L, "sauer_editface",       lua_editface);
    lua_register(L, "sauer_edittex",        lua_edittex);
    lua_register(L, "sauer_editent",        lua_editent);
    lua_register(L, "sauer_getcubeinfo",    lua_getcubeinfo);
    lua_register(L, "sauer_getsel",         lua_getsel);
    lua_register(L, "sauer_setsel",         lua_setsel);
    lua_register(L, "sauer_vcolor",         lua_vcolor);
    lua_register(L, "sauer_hashstring",     lua_hashstring);
    lua_register(L, "sauer_loadimage",      lua_loadimage);
    lua_register(L, "sauer_lookupcube",     lua_lookupcube);
    lua_register(L, "sauer_getplayerinfo",  lua_getplayerinfo);
    lua_register(L, "sauer_setplayerinfo",  lua_setplayerinfo);

    int status = luaL_loadfile(L, "faned/init.lua");
    int result = 0;

    if(status == LUA_OK) result = lua_pcall(L, 0, LUA_MULTRET, 0);
    else lua_error(L, status);

    conoutf("\f9FanEd\f7::LuaJIT: Version \f9%s \f7by \f9%s", LUA_RELEASE, LUA_AUTHORS);

    return result;
}

ICOMMAND(initlua, "", (), init_lua(););
ICOMMAND(resetlua, "", (), { lua_close(L); init_lua(); });

