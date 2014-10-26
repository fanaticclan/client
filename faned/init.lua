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

function lua_N_DIED(vcn, acn, frags, tfrags) end

function lua_N_EDITMODE(cn, val) end

function lua_N_SAYTEAM(cn, text) end

function lua_N_SERVCMD(cn, text)
     servcmd(text)
end

function lua_N_SHOTFX(cn, gun, to1, to2, to3) end

function lua_N_TEXT(cn, text)
    chatcmd(cn, text)
end

--[[

    Functions

--]]

function build_cube()
    for i = -200, 200 do
        for j = -200, 200 do
            k = 100+math.sin(math.sqrt(i^2+j^2)/20)*128
            sauer_build(512+i, 512+j, 512+k, 1, 0x80808080, 0x80808080, 0x80808080, 1, 1, 1, 1, 1, 1)
        end
    end
    sauer("recalc");
end

function calc(expression)
    fc = loadstring("return " .. expression)
    sauer("echo \f9FanEd\f7::LuaJIT: Result: " .. fc() .. ".")
end

function dec_to_hex(a)
    local b, i = {},0
    while i < 8 do
        i = i+1
        a,c = math.floor(a/16),(a%16)
        b[i] = c
    end
    return b
end

function detectviewtex(cn)
    Cpi = math.pi/180
    pitch = sauer_getplayerinfo(cn, 4)*Cpi; yaw = (sauer_getplayerinfo(cn, 5)+90.0)*Cpi;
    vx = math.cos(pitch)*math.cos(yaw); vy = math.cos(pitch)*math.sin(yaw); vz = math.sin(pitch);
    return sauer_getcubeinfo(cn,vx,vy,vz,0,5)
end

function detectdistdata(cn)
    Cpi = math.pi/180
    pitch = sauer_getplayerinfo(cn, 4)*Cpi; yaw = (sauer_getplayerinfo(cn, 5)+90.0)*Cpi;
    vx = math.cos(pitch)*math.cos(yaw); vy = math.cos(pitch)*math.sin(yaw); vz = math.sin(pitch);
    return sauer_getcubeinfo(cn,vx,vy,vz,0,-2),sauer_getcubeinfo(cn,vx,vy,vz,0,5)
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

function test_edit()
    local mycn = tonumber(sauer("getclientnum"));
    local x, y, z;
    x, y, z = sauer_getplayerinfo(mycn, 1, 2, 3);

    local h = 4
    local n = 400

    local grid;
    local grid0 = 8;

    local i, j, k, l;
    local x1, y1, z1, a, b, c

    grid = grid0
    h = h*2

    for i = 0, n do
        for j = 0,n do            
            x1 = x+i*grid
            y1 = y+j*grid
            z1 = math.random(h)
            c = math.random(10)
            if c > 5 then a = 0;
            b = math.random(h)      
            else
                a = math.random(z1);
                b = math.random(z1);
                if b > a then a, b = b, a end
            end            
            for k = a, b do
                sauer_build(x1, y1, z+k*grid, grid, 0x80808080, 0x80808080, 0x80808080, 1, 1, 1, 1, 1, 1)
            end
        end
    end
    
    grid = grid0*2
    h = h/2
    n = n/2
    
    for i = 0, n do
        for j = 0, n do            
            x1 = x+i*grid
            y1 = y+j*grid
            z1 = math.random(h)
            c = math.random(10)
            if c > 5 then a = 0;
            b = math.random(h)    
            else
                a = math.random(z1);
                b = math.random(z1);
                if b > a then a, b = b, a end
            end            
            for k = a,b do
                sauer_build(x1, y1, z+k*grid, grid, 0x80808080, 0x80808080, 0x80808080, 1, 1, 1, 1, 1, 1)
            end
        end
    end
end

function test_randomname()    
    local result = ""
    local i
    for i = 1,10 do
        result = result .. string.char(97+math.random(30))
    end
    sauer("name " .. result)
end

function test_stairs()
    ox, oy, oz, sx, sy, sz, grid = sauer_getsel(0, 1, 2, 3, 4, 5, 6)
    mycn = sauer("getclientnum");
    Cpi = math.pi/180.;
    pitch = sauer_getplayerinfo(mycn, 4)*Cpi;
    yaw = (sauer_getplayerinfo(mycn, 5)+90.0)*Cpi;
    vx = math.cos(pitch)*math.cos(yaw);
    vy = math.cos(pitch)*math.sin(yaw);
    vz = math.sin(pitch);
    pos = { sauer_getplayerinfo(mycn, 1, 2, 3) }
    ox, oy, oz = pos[1], pos[2], pos[3];
    local grid = 4
    for i = 1, 200 do
        sauer_editface(ox+vx*grid*i, oy+vy*grid*i, oz+vz*grid*i, 1, 1, 1, grid, 0, 2, 0, 2, 5, 0, -1, 1)
    end
end

function test_target()
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

function test_textures()
    local grid = 4
    for i = 1, 20 do
        for j = 1, 20 do
            sauer_edittex(512+i*grid, 286+j*grid, 512-grid, 1, 1, 1, grid, 0, 2, 0, 2, 5, 2, i*20+j, 1)
        end
    end
end

function tigerhash(text)
    local str_len = string.len(text)
    return sauer_hashstring(text, 49)
end

function vec_len(a)
    return math.sqrt(a[1]^2+a[2]^2+a[3]^2)
end

function vec_norm (a)
    s = math.sqrt(a[1]^2+a[2]^2+a[3]^2);
    if s == 0 then s = 1 end
    for i = 1,3 do a[i] = a[i] / s end
end

function vec_sub(a,b,c)
    if a[1] == nil then return end
    for i = 1,3 do c[i] = a[i]-b[i] end
end

