-- Fanatic Edition: entanim.lua

entanim = {}
 
entanim.ox = 512
entanim.oy = 512
entanim.oz = 512
 
entanim.num = 16
entanim.width = 14.5
entanim.spacing = 16
entanim.delay = 50
entanim.speed = 1000
entanim.rotatespeed = .5
entanim.graphspeed = 1
entanim.run = false
 
entanim.colcycles = {}
entanim.colcycles[1] = {  0,  1,  0 }
entanim.colcycles[2] = { -1,  0,  0 }
entanim.colcycles[3] = {  0,  0,  1 }
entanim.colcycles[4] = {  0, -1,  0 }
entanim.colcycles[5] = {  1,  0,  0 }
entanim.colcycles[6] = {  0,  0, -1 }
entanim.col = { 15, 0, 0 }
entanim.curcycle = 1
entanim.runcol = true

entanim.enttype = 5 -- Help: 5 = particles, 2 = mapmodels;
entanim.entattr = {}
entanim.entattr[1] = 13
entanim.entattr[2] = 273
entanim.entattr[3] = 32
entanim.entattr[4] = 0x0F0
entanim.entattr[5] = 100
 
local bit = require("bit")
 
function entanim.render(time)
    local d = entanim.width + entanim.spacing
    local tim = time/entanim.speed
    local fi = math.pi * (1 + math.cos(tim))
    local x, y, z, dx, dy, dz = 0, 0, 0, 0, 0, 0
    for j = 1, entanim.num do
        for i = 1, entanim.num do
            x = (i - entanim.num / 2) * d
            y = (j - entanim.num / 2) * d
            z = math.cos(math.sqrt((i - entanim.num / 2)^2 + (j - entanim.num/2)^2) * .4 + (tim * entanim.graphspeed) * 2) * 64
            local r = math.sqrt(x^2 + y^2)
            dx = x * math.cos(tim * entanim.rotatespeed) - y * math.sin(tim * entanim.rotatespeed)
            dy = y * math.cos(tim * entanim.rotatespeed) + x * math.sin(tim * entanim.rotatespeed)
            --dz = x * math.sin(fi) + z * math.cos(fi)
            sauer_editent((j * entanim.num) + i, dx + entanim.ox, dy + entanim.oy, z + entanim.oz, entanim.enttype, entanim.entattr[1], entanim.entattr[2], entanim.entattr[3], entanim.entattr[4], entanim.entattr[5], 1)
        end
    end
end
 
function entanim.loop()
    if entanim.run then
        sauer("sleep "..entanim.delay.." [ lua entanim.loop() ]")
        if entanim.runcol then
            local donextcycle = false
            for i = 1, 3 do
                entanim.col[i] = entanim.col[i] + entanim.colcycles[entanim.curcycle][i]
                if (entanim.col[i] == 15 and entanim.colcycles[entanim.curcycle][i] == 1) or
                   (entanim.col[i] == 0 and entanim.colcycles[entanim.curcycle][i] == -1) then
                    donextcycle = true
                end
            end
            if donextcycle then
                entanim.curcycle = entanim.curcycle + 1
                if entanim.curcycle > #entanim.colcycles then entanim.curcycle = 1 end
            end
            entanim.entattr[4] = rgb2int(entanim.col[1], entanim.col[2], entanim.col[3])
        end
        entanim.render(sauer("getmillis"))
    end
end
 
function entanim.start()
    entanim.run = true
    entanim.loop()
end
 
function entanim.stop()
    entanim.run = false
end
 
function rgb2int(r, g, b)
    return (r * 256) + (g * 16) + b
end

