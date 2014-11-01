-- Fanatic Edition: importimage.lua
-- importimagetexture by AC
-- importimagevcolor by bum

function texavggen()
    local width, height, l, filename = 100, 100

    local file, err = io.open("faned_texavg.dat", "w")
    if file == nil then
        sauer("echo \f9FanEd\f7::LuaJIT: \f3#error: \f7failed to load texture data")
        return
    end
    
    for l = 1, 1027 do
        filename = "packages/"..sauer("gettexname "..l)
        if string.len(filename) >= 127 then filename = string.sub(filename, 126) end
        
        local k = { 0, 0, 0 }
        pixels = sauer_loadimage(filename, width, height)
        if pixels == nil then
            sauer("echo \f9FanEd\f7::LuaJIT: \f3#error: \f7failed to load image")
            return
        end
        for i = 1, width do
            for j = 1, height do
                for p = 1, 3 do
                    k[p] = k[p] + math.floor(pixels[(j - 1) + ((i - 1) * height)][p])
                end
            end
        end

        for i = 1, 3 do
            k[i] = k[i] / (width * height)
        end
        file:write(l.." "..k[1].." "..k[2].." "..k[3].."\n")
    end
    file:close()
end

texavg = {};

function texavgload()
    local file = io.open("faned_texavg.dat","r")
    local lined = {}
    texavg = {}
    local str
    if file then
        for line in file:lines() do
            lined = {};
            for str in string.gmatch(line, "%S+") do lined[#lined + 1] = str end
            texavg[#texavg + 1] = lined
        end
        sauer("echo \f9FanEd\f7::LuaJIT: \f0#success: \f7texavg loaded")
        file:close()
    end
end

function match_tex(R, G, B)
    local i, j, k, dist
    dist = 99999999999999
    j = 1
    for i = 1, #texavg do
        k = (R - texavg[i][2])^2 + (G - texavg[i][3])^2 + (B - texavg[i][4])^2
        if k < dist then
            dist = k
            j = texavg[i][1]
        end
    end 
    return j
end

function buildimagetexture(filename, w, h)
    local ox, oy, oz, grid = sauer_getsel(0, 1, 2, 6)
    local k, index = 0, 0
    pixels = sauer_loadimage(filename, w, h)
    if pixels == nil then
        sauer("echo \f9FanEd\f7::LuaJIT: \f3#error: \f7failed to load image")
        return
    end
    for i = 0, w - 1 do
        for j = 0, h - 1 do
            index = j + (i * h)
            k = match_tex(pixels[index][1], pixels[index][2], pixels[index][3])
            sauer_build(ox + (i * grid), oy + (j * grid), oz + grid, grid, 0x80808080, 0x80808080, 0x80808080, k, k, k, k, k, k)
        end
    end
    sauer("recalc")
end

function importimagevcolor(filename, w, h, sr, sg, sb, sa)
    local ox, oy, oz, grid, orient = sauer_getsel(0, 1, 2, 6, 11)
    local all = sauer("$allfaces")
    if all == 1 then orient = -1 end

    pixels = sauer_loadimage(filename, w, h)
    if pixels == nil then
        sauer("echo \f9FanEd\f7::LuaJIT: \f3#error: \f7failed to load image")
        return
    end

    for x = 0, w - 1 do
        for y = 0, h - 1 do
            local index = y + (x * h)
            local r = pixels[index][1]
            local g = pixels[index][2]
            local b = pixels[index][3]
            if sr ~= nil then r = (r + sr) / 2 end
            if sg ~= nil then g = (g + sg) / 2 end
            if sb ~= nil then b = (b + sb) / 2 end
            if sa ~= nil and sa < 255 then
                sa = sa / 255
                local dif = 1 - sa
                r = (r * sa) + (255 * dif)
                g = (g * sa) + (255 * dif)
                b = (b * sa) + (255 * dif)
            end
            r = r / 255
            g = g / 255
            b = b / 255
            sauer_vcolor(ox + (x * grid), oy + (y * grid), oz, 1, 1, 1, grid, r, g, b, orient);
        end
    end
end

