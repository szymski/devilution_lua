#include "api.h"
#include "../types.h"

#include <fcntl.h>

lua_State *L;

void api_register_functions();

void api_init()
{
	freopen("//./NUL", "w", stdout);
	setvbuf(stdout, NULL, _IONBF, 0);

	AllocConsole();
	void *hStd = GetStdHandle(STD_OUTPUT_HANDLE);
	int fdStd = _open_osfhandle((intptr_t)hStd, _O_TEXT);
	_dup2(fdStd, fileno(stdout));
	SetStdHandle(STD_OUTPUT_HANDLE, (HANDLE)_get_osfhandle(fileno(stdout)));
	_close(fdStd);

	printf("Initializing Lua API\n");

	L = luaL_newstate();
	luaL_openlibs(L);
	//lua_setglobal(L, "_G");

	api_register_functions();

	api_load_file("init.lua");
}

void api_load_file(const char *filename)
{
	int status = luaL_loadfile(L, filename);
	if (status)
		printf("Couldn't load lua file: %s\n", lua_tostring(L, -1));

	int result = lua_pcall(L, 0, 0, 0);
	if (result)
		printf("Lua error: %s\n", lua_tostring(L, -1));
}

void api_call_function()
{
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		return;
	}

	int result = lua_pcall(L, 0, 0, 0);
	if (result)
		printf("Lua error: %s\n", lua_tostring(L, -1));
}

extern "C" int _cdecl l_msg(lua_State *L)
{
	const char *str = luaL_checkstring(L, 1);
	printf("%s", str);
	return 0;
}

extern "C" int _cdecl l_PlaySFX(lua_State *L)
{
	int id = luaL_checkinteger(L, 1);
	PlaySFX(id);
	return 0;
}

extern "C" int _cdecl l_PrintGameStr(lua_State *L)
{
	auto x = static_cast<int>(luaL_checknumber(L, 1));
	auto y = static_cast<int>(luaL_checknumber(L, 2));
	auto str = luaL_checkstring(L, 3);
	auto color = luaL_checknumber(L, 4);

	PrintGameStr(x, y, (char*)str, color);

	return 0;
}

extern "C" int _cdecl l_DrawLine(lua_State *L)
{
	auto x0 = static_cast<int>(luaL_checknumber(L, 1));
	auto y0 = static_cast<int>(luaL_checknumber(L, 2));
	auto x1 = static_cast<int>(luaL_checknumber(L, 3));
	auto y1 = static_cast<int>(luaL_checknumber(L, 4));
	auto col = static_cast<int>(luaL_checknumber(L, 5));

	DrawLine(x0, y0, x1, y1, col);

	return 0;
}

#define REGISTER_ENUM(NAME) \
	lua_pushnumber(L, NAME);   \
	lua_setglobal(L, #NAME);

#define REGISTER_FUNC(NAME)  \
	lua_pushcfunction(L, l_ ## NAME); \
	lua_setglobal(L, #NAME);

void api_register_functions()
{
	REGISTER_FUNC(msg);

	REGISTER_FUNC(PlaySFX);

	// Drawing

	REGISTER_ENUM(COL_WHITE);
	REGISTER_ENUM(COL_BLUE);
	REGISTER_ENUM(COL_RED);
	REGISTER_ENUM(COL_GOLD);

	REGISTER_ENUM(PAL8_BLUE);
	REGISTER_ENUM(PAL8_RED);
	REGISTER_ENUM(PAL8_YELLOW);
	REGISTER_ENUM(PAL8_ORANGE);
	REGISTER_ENUM(PAL16_BEIGE);
	REGISTER_ENUM(PAL16_BLUE);
	REGISTER_ENUM(PAL16_YELLOW);
	REGISTER_ENUM(PAL16_ORANGE);
	REGISTER_ENUM(PAL16_RED);
	REGISTER_ENUM(PAL16_GRAY);

	REGISTER_FUNC(PrintGameStr);
	REGISTER_FUNC(DrawLine);
}
