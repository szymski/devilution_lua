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

    draw.printGameStr(5, 15, "Lua API Test", COL_WHITE)
    draw.printGameStr(5, 35, level.getType() == DTYPE_TOWN and "Town" or "Not town", COL_WHITE)
    
    local x, y = mouse.getPos()
    draw.printGameStr(5, 55, "" .. x .. " " .. y, COL_BLUE)

    draw.printGameStr(5, 75, "Gold: " .. myPly:getGold(), COL_GOLD)

    a = a + 1

    draw.drawLine(0, 0, 300 + math.cos(a * 0.05) * 200, 300 + math.sin(a * 0.05) * 200, PAL8_ORANGE)
end

function Tick()
    if not myPly then
        return
    end

    if myPly:getGold() == 50 then
        local ply = myPly
        myPly = nil

        timer.simple(2, function()
            msg("2")
            
        end)

        timer.simple(1, function()
            msg("1")

            timer.simple(2, function()
                msg("3")
                ply:kill()
            end)
        end)
    end
end