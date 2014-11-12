-- Fanatic Edition: init.lua

--[[

    Init

--]]

function execluadir(directory)
    sauer("loopfiles f "..directory.." lua [execlua (format \"%1/%2.lua\" "..directory.." $f)]")
end

execluadir("faned/scripts/lua")

--[[

    Events

--]]

function lua_N_DAMAGE(acn, tcn, damage, armour, health) end

function lua_N_EVENT(event, arg1, arg2, arg3, arg4) end

function lua_N_DIED(acn, vcn, frags, tfrags) end

function lua_N_EDITMODE(cn, val) end

function lua_N_SAYTEAM(cn, text) end

function lua_N_SERVCMD(cn, text) end

function lua_N_SHOTFX(cn, gun, to1, to2, to3) end

function lua_N_TEXT(cn, text) end

--[[

    Functions

--]]

function calc(expression)
    fc = loadstring("return " .. expression)
    sauer("echo \f9FanEd\f7::LuaJIT: Result: " .. fc() .. ".")
end

function dectohex(a)
    local b, i = {},0
    while i < 8 do
        i = i+1
        a,c = math.floor(a/16),(a%16)
        b[i] = c
    end
    return b
end

function detectcube()
    Cpi = math.pi/180.;
    mycn = sauer("getclientnum");  
    pitch = sauer_getplayerinfo(mycn, 4)*Cpi;
    yaw = (sauer_getplayerinfo(mycn, 5)+90.0)*Cpi;
    vx = math.cos(pitch)*math.cos(yaw);
    vy = math.cos(pitch)*math.sin(yaw);
    vz = math.sin(pitch);
    sauer_echo(sauer_getcubeinfo(mycn,vx,vy,vz,0,-2))
    sauer_echo(sauer_getcubeinfo(mycn,vx,vy,vz,0,5))
    tex = sauer_getcubeinfo(mycn,0,0,-1,0,2)
    sauer_echo(tex)
end

function detectdist(cn)
    Cpi = math.pi/180
    pitch = sauer_getplayerinfo(cn, 4)*Cpi; yaw = (sauer_getplayerinfo(cn, 5)+90.0)*Cpi;
    vx = math.cos(pitch)*math.cos(yaw); vy = math.cos(pitch)*math.sin(yaw); vz = math.sin(pitch);
    return sauer_getcubeinfo(cn,vx,vy,vz,0,-2),sauer_getcubeinfo(cn,vx,vy,vz,0,5)
end

function detecttex(cn)
    Cpi = math.pi/180
    pitch = sauer_getplayerinfo(cn, 4)*Cpi; yaw = (sauer_getplayerinfo(cn, 5)+90.0)*Cpi;
    vx = math.cos(pitch)*math.cos(yaw); vy = math.cos(pitch)*math.sin(yaw); vz = math.sin(pitch);
    return sauer_getcubeinfo(cn,vx,vy,vz,0,5)
end

function pitchyawfromvec(a)
    local r = vec_len(a);
    if (r == 0) then return; end
    local yaw =  math.atan(-a[1] / a[2])
    if a[2] < 0 then yaw = yaw+math.pi end
    if (a[1] > 0 and a[2] > 0) then yaw = yaw + 2*math.pi end
    return math.asin(a[3]/r),yaw;
end

function sauer_echo(text)
    sauer("echo \f7".. text)
end

function sauer_print(text)
    sauer("echo \f7".. text)
end

function sauer_say(text)
    local mycn = sauer("getclientnum")
    lua_N_TEXT(mycn, text)
end

function tigerhash(text)
    local str_len = string.len(text)
    return sauer_hashstring(text, 49)
end

function veclen(a)
    return math.sqrt(a[1]^2+a[2]^2+a[3]^2)
end

function vecnorm(a)
    s = math.sqrt(a[1]^2+a[2]^2+a[3]^2);
    if s == 0 then s = 1 end
    for i = 1,3 do a[i] = a[i] / s end
end

function vecsub(a,b,c)
    if a[1] == nil then return end
    for i = 1,3 do c[i] = a[i]-b[i] end
end

