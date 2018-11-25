msg("Hello Lua world!")

hook.add("StartGame", "", function()
    msg("Game started")
    myPly = player.getLocalPlayer()
end)

a = 0
hook.add("PostDrawGame", "", function()
    if not myPly then
        return
    end

    draw.printGameStr(5, 15, "Lua API Test", COL_WHITE)
    draw.printGameStr(5, 35, myPly:getLevel() == DTYPE_TOWN and "Town" or "Not town", COL_WHITE)
    
    local x, y = mouse.getPos()
    draw.printGameStr(5, 55, "" .. x .. " " .. y, COL_BLUE)

    draw.printGameStr(5, 75, "Gold: " .. myPly:getGold(), COL_GOLD)
    draw.printGameStr(5, 95, "Time: " .. timer.getTime(), COL_WHITE)

    a = a + 1

    draw.drawLine(0, 0, 300 + math.cos(a * 0.05) * 200, 300 + math.sin(a * 0.05) * 200, PAL8_ORANGE)
end)

nextSpawn = 0

hook.add("Tick", "", function()
    if myPly:getGold() == 50 then
        hook.remove("Tick", "")

        timer.simple(3, function()
            myPly:kill()
        end)
    end

    if myPly then
        if timer.getTime() > nextSpawn then
            nextSpawn = timer.getTime() + 0.1

            local x, y = myPly:getPos()
            local monster = monster.spawn(x, y, 0, 1, true)

            if monster then
                msg("Spawned monster " .. monster:getName())
            end
        end
    end
end)

hook.add("PlayerDie", "", function(ply)
    msg(ply:getName() .. " died")
end)

hook.add("LevelChange", "", function(ply, lvl)
    msg(ply:getName() .. " changed level to " .. lvl)
end)

-- hook.add("SetPlayerSpeed", "", function(ply)
--     if ply:getLevel() == DTYPE_TOWN then
--         return 2048
--     end
-- end)

-- hook.add("InitMonster", "", function(monster)
--     msg("Initialized monster " .. monster:getName())

--     timer.simple(10, function()
--         msg("Killing " .. monster:getName())
--         monster:kill()
--     end)
-- end)