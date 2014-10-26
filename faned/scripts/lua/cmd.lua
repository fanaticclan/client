-- Fanatic Edition: cmd.lua

--[[

     Instructions

     1. Enable chat command parsing: /lua setchatcmd.guard=0;
     2. Function is parsing commands in following syntax: clientname !command arguments;
     3. Disable chat command parsing: /lua setchatcmd.guard=1;

--]]

setchatcmd = {}
setchatcmd.guard = 0
 
function chatcmd(cn, text)
    if setchatcmd.guard == 1 then
        return
    end

    local args = {}
    local argc = 0
    local word

    for word in string.gmatch(text, "%S+") do
        table.insert(args, word)
        argc = argc + 1
    end

    if args[1] == sauer("getclientname") then
        if args[2] == "!build" then
            sauer("say Build: (getbuildversion)") 
        end
        if args[2] == "!calc" then
            local cmd = string.sub(text, (sauer("(+ (strlen $getclientname) 8)")))
            if string.find(cmd, "debug.") ~= nil or
               string.find(cmd, "io.") ~= nil or
               string.find(cmd, "os.") ~= nil or
               string.find(cmd, "sauer(") ~= nil then
               sauer("say Invalid command.")
               return
            else
                fc = loadstring("return " .. cmd)
                sauer("say Result: " .. fc())
            end
        end
	if args[2] == "!combatgear" then
            sauer("_gotodist = $gotodist; gotodist 0; edittoggle; goto " .. cn .. "; sleep 0 [edittoggle; combatgear; gotodist $_gotodist]")
	end
        if args[2] == "!come" then sauer("edittoggle; goto " .. cn .. "; sleep 0 edittoggle")
        end
        if args[2] == "!date" then
            sauer("say Date: (date %Y-%m-%d)") 
        end
        if args[2] == "!help" then
            sauer("say Commands::Chat: !build, !calc, !come, !combatgear, !date, !say, !time and !uptime.")
        end
        if args[2] == "!say" then
            local cmd = string.sub(text, (sauer("(+ (strlen $getclientname) 7)")))
            if string.find(cmd, "debug.") ~= nil or
               string.find(cmd, "io.") ~= nil or
               string.find(cmd, "os.") ~= nil or
               string.find(cmd, "sauer(") ~= nil then
               sauer("say Invalid command.")
               return
            end
            sauer("say \"" .. cmd .. "\"") 
        end
        if args[2] == "!time" then
            sauer("say Time: (date %H:%M:%S)") 
        end
        if args[2] == "!uptime" then
            sauer("say Uptime: (uptime)") 
        end
    end
end

--[[

     Instructions

     1. Enable server command parsing: /lua setservcmd.guard=0;
     2. Disable server command parsing: /lua setservcmd.guard=1;

--]]

setservcmd = {}
setservcmd.guard = 0
 
function servcmd(text)
    if setchatcmd.guard == 1 then
        return
    end
    if string.find(text, "!build") then
        sauer("say Build: (getbuildversion)") 
    end
    if string.find(text, "!date") then
        sauer("say Date: (date %Y-%m-%d)") 
    end
    if string.find(text, "!help") then
        sauer("say Commands::IRC: !build, !date, !help, !time and !uptime.")
    end
    if string.find(text, "!time") then
        sauer("say Time: (date %H:%M:%S)") 
    end
    if string.find(text, "!uptime") then
        sauer("say Uptime: (uptime)") 
    end
end

