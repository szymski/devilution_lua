print("Initializing console")

console = console or {
    opened = false,
    inputText = "",
    lines = { },
    _print = print,
    commands = { },
    cursorPos = 1,
    prevBuffer = { },
    prevSelected = 0,
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

function msgC(text, color)
    console._print(text)
    for k, v in pairs(tostring(text):split("\n")) do
        console.addLine(v, color)
    end
end

function print(...)
    console._print(...)

    local result = { }
    for k, v in pairs({ ... }) do
        result[#result + 1] = tostring(v)
    end

    console.addLine(table.concat(result, ", "), COL_WHITE)
end

local VKEY_LEFT = 37
local VKEY_UP = 38
local VKEY_RIGHT = 39
local VKEY_DOWN = 40

local lastCursorMove = 0

hook.add("KeyPress", "Console_KeyPress", function(code, char)    
    if console.opened then
        if code == VKEY_LEFT then
            console.cursorPos = math.max(0, console.cursorPos - 1)
            lastCursorMove = timer.getTime()
        elseif code == VKEY_RIGHT then
            console.cursorPos = math.min(console.cursorPos + 1, #console.inputText)
            lastCursorMove = timer.getTime()
        elseif code == VKEY_UP then
            console.prevSelected = console.prevSelected - 1
            if console.prevSelected < 1 then
                console.prevSelected = #console.prevBuffer
            end

            console.inputText = console.prevBuffer[console.prevSelected]
            console.cursorPos = #console.inputText
        elseif code == VKEY_DOWN and console.prevSelected ~= 0 then
            console.prevSelected = console.prevSelected + 1
            if console.prevSelected > #console.prevBuffer then
                console.prevSelected = 0
                console.inputText = ""
                console.cursorPos = 0
            else
                console.inputText = console.prevBuffer[console.prevSelected]
                console.cursorPos = #console.inputText
            end
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
                lastCursorMove = timer.getTime()
            end
        elseif code == 13 then
            console.runCommand(console.inputText)
            console.inputText = ""
            console.cursorPos = 0
            lastCursorMove = timer.getTime()
        else
            console.inputText = console.inputText:sub(1, console.cursorPos) .. char .. console.inputText:sub(console.cursorPos + 1)
            console.cursorPos = console.cursorPos + 1
            lastCursorMove = timer.getTime()
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
    draw.drawRectTransparent(-1, openOffset + totalHeight - lineHeight + offset, 641, lineHeight + offset, 252)
    
    draw.printGameStr(offset, openOffset + totalHeight + 3, console.inputText, COL_WHITE)
    if (timer.getTime() - lastCursorMove) % 1 < 0.5 then
        -- local cursorStr = console.inputText:sub(1, console.cursorPos) .. "_"
        -- draw.printGameStr(offset, openOffset + totalHeight + 3, cursorStr, COL_WHITE)

        draw.drawRect(offset + draw.getTextWidth(console.inputText:sub(1, console.cursorPos)) - 1, openOffset + totalHeight + 3 - 10, 1, 10, 255)
    end

    for k, v in pairs(console.lines) do
        draw.printGameStr(offset, openOffset + openAnim + lineHeight * k, v[1], v[2])
    end
end)

function console.runCommand(cmd)
    console.prevSelected = 0

    if #console.prevBuffer == 0 or console.prevBuffer[#console.prevBuffer] ~= cmd then
        if #console.prevBuffer > 10 then
            table.remove(console.prevBuffer, 1)
        end

        console.prevBuffer[#console.prevBuffer + 1] = cmd
    end

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
        msgC("No such command: " .. name, COL_RED)
    end
end

console.registerCommand("reload", function()
    msgC("Reloading everything", COL_BLUE)

    for k, v in pairs(package.loaded) do
        package.loaded[k] = nil
    end

    require("init")
end)

console.registerCommand("test", function(cmd, ...)
    msgC("Args: ")
    for k, v in pairs({ ... }) do
        msgC("" .. v)
    end
end)

console.registerCommand("kill", function()
    player.getLocalPlayer():kill()
end)

console.registerCommand("getpos", function()
    local x, y = player.getLocalPlayer():getPos()
    msgC("" .. x .. " " .. y)
end)

console.registerCommand("setpos", function(cmd, x, y)
    local nX, nY = tonumber(x), tonumber(y)

    if not nX or not nY then
        msgC("Invalid position", COL_RED)
        return
    end

    player.getLocalPlayer():setPos(nX, nY)
end)

console.registerCommand("spawnmonster", function(cmd, type)
    local type = tonumber(type)

    if not type or type <= 0 then
        msgC("Invalid type", COL_RED)
        return
    end

    local x, y = player.getLocalPlayer():getPos()

    local m = monster.spawn(x, y, 0, type, true)

    if m then
        msgC("Spawned monster " .. m:getName())
    else
        msgC("Spawning monster failed", COL_ERROR)
    end
end)

console.registerCommand("lua", function(cmd, ...)
    local command = table.concat({ ... }, " ")
    msgC("Running lua")

    xpcall(loadstring(command), function(err)
        msgC("LUA ERROR: " .. err, COL_RED)
    end)
end)