msg("Initializing console", COL_GOLD)

console = console or {
    opened = false,
    inputText = "",
    lines = { },
    _msg = msg,
    commands = { },
    cursorPos = 1,
}

local openAnim = console.opened and 1 or 0

local visibleLines = 10

function console.addLine(text, color)
    if #console.lines >= visibleLines then
        table.remove(console.lines, 1)
    end
    console.lines[#console.lines + 1] = { text, color or COL_WHITE }
end

function console.registerCommand(name, callback)
    console.commands[name] = callback
end

function msg(text, color)
    console._msg(text)
    console.addLine(text, color)
end

local VKEY_LEFT = 37
local VKEY_UP = 38
local VKEY_RIGHT = 39
local VKEY_DOWN = 40

hook.add("KeyPress", "Console_KeyPress", function(code, char)    
    if console.opened then
        if code == VKEY_LEFT then
            console.cursorPos = math.max(0, console.cursorPos - 1) 
        elseif code == VKEY_RIGHT then
            console.cursorPos = math.min(console.cursorPos + 1, #console.inputText + 1) 
        end

        return false
    end
end)

hook.add("CharPress", "Console_CharPress", function(code, char)
    if code == 96 or char == "]" then
        console.opened = not console.opened
        return false
    end

    if console.opened then
        if code == 8 then
            if #console.inputText > 0 and console.cursorPos > 0 then
                console.inputText = console.inputText:sub(1, console.cursorPos - 1) .. console.inputText:sub(console.cursorPos + 1)
                console.cursorPos = console.cursorPos - 1
            end
        elseif code == 13 then
            console.runCommand(console.inputText)
            console.inputText = ""
            console.cursorPos = 1
        else
            console.inputText = console.inputText:sub(1, console.cursorPos) .. char .. console.inputText:sub(console.cursorPos + 1)
            console.cursorPos = console.cursorPos + 1
        end

        return false
    end
end)

local yOffset = 30
local offset = 4
local lineHeight = 15

local animSpeed = 0.1

hook.add("PostDrawGame", "Console_Draw", function()
    if not console.opened and openAnim < 0.01 then
        return
    end

    if console.opened then
        openAnim = math.min(1, openAnim + animSpeed)
    else
        openAnim = math.max(0, openAnim - animSpeed)
    end

    local totalHeight = (visibleLines + 1) * lineHeight + offset

    local openOffset = -totalHeight * (1 - openAnim)

    draw.drawRectTransparent(0, openOffset, 640, totalHeight, 0)
    draw.drawRectTransparent(-1, openOffset + totalHeight - lineHeight + offset, 641, lineHeight + offset, 1)
    
    draw.printGameStr(offset, openOffset + totalHeight + 3, console.inputText, COL_WHITE)
    if true or timer.getTime() % 1 < 0.5 then
        local cursorStr = console.inputText:sub(1, console.cursorPos) .. "_"
        draw.printGameStr(offset, openOffset + totalHeight + 3, cursorStr, COL_WHITE)
    end

    for k, v in pairs(console.lines) do
        draw.printGameStr(offset, openOffset + openAnim + lineHeight * k, v[1], v[2])
    end
end)

function console.runCommand(cmd)
    local split = cmd:trim():split(" ")
    local name = split[1] or ""
    table.remove(split, 1)
    
    local params = { }
    for k, v in pairs(split) do
        if v ~= "" then
            params[#params + 1] = v
        end
    end

    local callback = console.commands[name]
    if callback then
        callback(cmd, unpack(params))
    else
        msg("No such command: " .. name, COL_RED)
    end
end

console.registerCommand("reload", function()
    msg("Reloading everything", COL_BLUE)

    for k, v in pairs(package.loaded) do
        package.loaded[k] = nil
    end

    require("init")
end)

console.registerCommand("test", function(cmd, ...)
    msg("Args: ")
    for k, v in pairs({ ... }) do
        msg("" .. v)
    end
end)

console.registerCommand("kill", function()
    player.getLocalPlayer():kill()
end)

console.registerCommand("getpos", function()
    local x, y = player.getLocalPlayer():getPos()
    msg("" .. x .. " " .. y)
end)

console.registerCommand("setpos", function(cmd, x, y)
    local nX, nY = tonumber(x), tonumber(y)

    if not nX or not nY then
        msg("Invalid position", COL_RED)
        return
    end

    player.getLocalPlayer():setPos(nX, nY)
end)

console.registerCommand("spawnmonster", function(cmd, type)
    local type = tonumber(type)

    if not type or type <= 0 then
        msg("Invalid type", COL_RED)
        return
    end

    local x, y = player.getLocalPlayer():getPos()

    local m = monster.spawn(x, y, 0, type, true)

    if m then
        msg("Spawned monster " .. m:getName())
    else
        msg("Spawning monster failed", COL_ERROR)
    end
end)