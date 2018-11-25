msg("Hello Lua world!\n")

function OnStartGame()
    msg("Game started\n")
end

a = 0
function PostDrawGame()
    PrintGameStr(5, 15, "Lua API Test", COL_WHITE)
    PrintGameStr(5, 35, "Blue", COL_BLUE)
    PrintGameStr(5, 55, "Red", COL_RED)
    PrintGameStr(5, 75, "Gold", COL_GOLD)

    a = a + 1

    DrawLine(0, 0, 300 + math.cos(a * 0.05) * 200, 300 + math.sin(a * 0.05) * 200, PAL8_ORANGE)
end