-- Fanatic Edition: build.lua
-- build tests by AC

function test_build_a()
    for i = -200, 200 do
        for j = -200, 200 do
            k = 100+math.sin(math.sqrt(i^2+j^2)/20)*128
            sauer_build(512+i, 512+j, 512+k, 1, 0x80808080, 0x80808080, 0x80808080, 1, 1, 1, 1, 1, 1)
        end
    end
    sauer("recalc");
end

function test_build_b()
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
    sauer("recalc");
end

function test_build_c()
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

