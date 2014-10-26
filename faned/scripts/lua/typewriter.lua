-- Fanatic Edition: typewriter.lua

charpos = {};
printpos = {};
print_offset = 0;

function settypewriterchars()
    charpos = { sauer_getsel(0, 1, 2, 3, 4, 5, 6) }
end

function settypewriterprint()
    printpos = { sauer_getsel(0, 1, 2, 6, 5, 3) }
    printpos[3] = printpos[3] + printpos[5]*printpos[4]
end

function typechar(i, j)
    ox, oy, oz, grid, sz, sx = unpack(printpos)
    oz = oz - print_offset*grid
    sauer_copy(charpos[1]+(i-1)*charpos[7], charpos[2], charpos[3], 1, 1, 1, charpos[7], 0, 2, 0, 2, 5, 2)
    sauer_paste(ox+(j-1)*grid, oy, oz, 1, 1, 1, grid, 0, 2, 0, 2, 5, 2) 
end

typewriterdata = { 1, 0, "", 1 }

function typewriter(text)
    if text ~= nil then

        typewriterdata = { 1, string.len(text), text, 0 }; 

        if typewriter_blackboard == 1 then
            print_offset = print_offset + 1
            if print_offset>printpos[5] then print_offset = 1 end
            sauer_editface(printpos[1], printpos[2], printpos[3]-print_offset*printpos[4], printpos[6], 1, 1, printpos[4], 0, 2, 0, 2, 5, 0, 1, 1)
        end
        sauer("luatyp = []; luatyp = [lua typewriter(); sleep 20 [luatyp]]; luatyp")
        return
    end
    
    j = typewriterdata[1]
    
    if j>typewriterdata[2] then 
        sauer("luatyp = []")
        typewriterdata[4] = 1;
        return
    end

    i = string.byte(typewriterdata[3], j);

    --[[

    char scheme: abcdefghijklmnopqrstuvwxyz0123456789;

    -- ,  .  !  ?  /  \  "  :  %  ~   [  ]  (  )  <  >  -  +  *  =  |    _  '     0-9   a-z
    -- 44 46 33 63 47 92 34 58 37 126 91 93 40 41 60 62 45 43 42 61 124 95  39 36 48-57 97-122

    ]]--

    if i >= 97 and i <= 122 then i = i-97
    elseif i>=48 and i<=57 then i = i-48+26
    elseif i == 44 then i = 37
    elseif i == 46 then i = 38
    elseif i == 33 then i = 39
    elseif i == 63 then i = 40
    elseif i == 47 then i = 41
    elseif i == 92 then i = 42
    elseif i == 34 then i = 43
    elseif i == 58 then i = 44
    elseif i == 37 then i = 45
    elseif i == 126 then i = 46
    elseif i == 91 then i = 47
    elseif i == 93 then i = 48
    elseif i == 40 then i = 49
    elseif i == 41 then i = 50
    elseif i == 60 then i = 51
    elseif i == 62 then i = 52
    elseif i == 45 then i = 53
    elseif i == 43 then i = 54
    elseif i == 42 then i = 55
    elseif i == 61 then i = 56    
    elseif i ==124 then i = 57
    elseif i == 95 then i = 58
    elseif i == 39 then i = 59
    else i = 36 end
    i = i + 1

    typechar(i, j)
    typewriterdata[1] = typewriterdata[1] + 1
end

