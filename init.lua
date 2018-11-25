msg("Hello Lua world!")

function OnStartGame()
    msg("Game started")
    myPly = player.getLocalPlayer()
end

a = 0
function PostDrawGame()
    if not myPly then
        return
    end

    draw.printGameStr(5, 15, "Lua API Test", COL_GOLD)
    draw.printGameStr(5, 35, level.getType() == DTYPE_TOWN and "Town" or "Not town", COL_WHITE)
    
    local x, y = mouse.getPos()
    draw.printGameStr(5, 55, "" .. x .. " " .. y, COL_BLUE)

    draw.printGameStr(5, 75, myPly:getName(), COL_RED)
    draw.printGameStr(5, 95, "" .. myPly:getHP(), COL_RED)

    a = a + 1

    draw.drawLine(0, 0, 300 + math.cos(a * 0.05) * 200, 300 + math.sin(a * 0.05) * 200, PAL8_ORANGE)
end

function Tick()
    if not myPly then
        return
    end

    myPly:setHP(myPly:getHP() + 1)
end