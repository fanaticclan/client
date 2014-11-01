-- Fanatic Edition: animate.lua

animate = {}

animate.ox = 512
animate.oy = 512
animate.oz = 512
 
animate.num = 16
animate.width = 14.5
animate.spacing = 16
animate.delay = 50
animate.speed = 1000
animate.rotatespeed = .5
animate.graphspeed = 1
animate.run = false
 
animate.colcycles = {}
animate.colcycles[1] = {  0,  1,  0 }
animate.colcycles[2] = { -1,  0,  0 }
animate.colcycles[3] = {  0,  0,  1 }
animate.colcycles[4] = {  0, -1,  0 }
animate.colcycles[5] = {  1,  0,  0 }
animate.colcycles[6] = {  0,  0, -1 }
animate.col = { 15, 0, 0 }
animate.curcycle = 1
animate.runcol = true

animate.enttype = 5 -- Help: 5 = particles, 2 = mapmodels;
animate.entattr = {}
animate.entattr[1] = 13
animate.entattr[2] = 273
animate.entattr[3] = 32
animate.entattr[4] = 0x0F0
animate.entattr[5] = 100
 
local bit = require("bit")
 
function animate.render(time)
    local d = animate.width + animate.spacing
    local tim = time/animate.speed
    local fi = math.pi * (1 + math.cos(tim))
    local x, y, z, dx, dy, dz = 0, 0, 0, 0, 0, 0
    for j = 1, animate.num do
        for i = 1, animate.num do
            x = (i - animate.num / 2) * d
            y = (j - animate.num / 2) * d
            z = math.cos(math.sqrt((i - animate.num / 2)^2 + (j - animate.num/2)^2) * .4 + (tim * animate.graphspeed) * 2) * 64
            local r = math.sqrt(x^2 + y^2)
            dx = x * math.cos(tim * animate.rotatespeed) - y * math.sin(tim * animate.rotatespeed)
            dy = y * math.cos(tim * animate.rotatespeed) + x * math.sin(tim * animate.rotatespeed)
            --dz = x * math.sin(fi) + z * math.cos(fi)
            sauer_editent((j * animate.num) + i, dx + animate.ox, dy + animate.oy, z + animate.oz, animate.enttype, animate.entattr[1], animate.entattr[2], animate.entattr[3], animate.entattr[4], animate.entattr[5], 1)
        end
    end
end
 
function animate.loop()
    if animate.run then
        sauer("sleep "..animate.delay.." [ lua animate.loop() ]")
        if animate.runcol then
            local donextcycle = false
            for i = 1, 3 do
                animate.col[i] = animate.col[i] + animate.colcycles[animate.curcycle][i]
                if (animate.col[i] == 15 and animate.colcycles[animate.curcycle][i] == 1) or
                   (animate.col[i] == 0 and animate.colcycles[animate.curcycle][i] == -1) then
                    donextcycle = true
                end
            end
            if donextcycle then
                animate.curcycle = animate.curcycle + 1
                if animate.curcycle > #animate.colcycles then animate.curcycle = 1 end
            end
            animate.entattr[4] = rgb2int(animate.col[1], animate.col[2], animate.col[3])
        end
        animate.render(sauer("getmillis"))
    end
end
 
function animate.start()
    animate.run = true
    animate.loop()
end
 
function animate.stop()
    animate.run = false
end
 
function rgb2int(r, g, b)
    return (r * 256) + (g * 16) + b
end

