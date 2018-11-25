#include "api.h"
#include "api_types.h"
#include "../types.h"

#include <fcntl.h>

sol::state lua;

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

	lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table, sol::lib::os);

	api_register_functions();

	api_load_file("init.lua");
}

void api_load_file(const char *filename)
{
	try {
		auto result = lua.safe_script_file(filename);
	} catch (const sol::error &e) {
		std::cout << "Lua error: " << e.what() << std::endl;
	}
}

void api_call_function(const char *name)
{
	try {
		sol::function f = lua[name];
		if (f.valid())
			f.call();
	} catch (const sol::error &e) {
		std::cout << "Lua error: " << e.what() << std::endl;
	}
}

#define REGISTER_ENUM(NAME) \
	lua[#NAME] = (int)NAME

void api_register_functions()
{
	lua["msg"] = [](std::string str) {
		std::cout << str << std::endl;
	};

	// Player library

	auto playerType = lua.new_usertype<api_PlayerStruct>("Player",
	    "getName", &api_PlayerStruct::getName,

	    "getHP", &api_PlayerStruct::getHP,
	    "setHP", &api_PlayerStruct::setHP);

	auto player = lua.create_table();
	lua["player"] = player;

	player["getLocalPlayer"] = []() {
		api_PlayerStruct ply{ myplr, plr[myplr] };
		return ply;
	};

	// Mouse library

	auto mouse
	    = lua.create_table();
	lua["mouse"] = mouse;

	mouse["getWorldPos"] = []() {
		return std::make_tuple(cursmx, cursmy);
	};

	mouse["getPos"] = []() {
		return std::make_tuple(MouseX, MouseY);
	};

	// Level library

	REGISTER_ENUM(DTYPE_TOWN);
	REGISTER_ENUM(DTYPE_CATHEDRAL);
	REGISTER_ENUM(DTYPE_CATACOMBS);
	REGISTER_ENUM(DTYPE_CAVES);
	REGISTER_ENUM(DTYPE_HELL);
	REGISTER_ENUM(DTYPE_NONE);

	auto level = lua.create_table();
	lua["level"] = level;

	level["getType"] = []() {
		return leveltype;
	};

	// Draw library

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

	auto draw = lua.create_table();
	lua["draw"] = draw;

	draw["printGameStr"] = [](double x, double y, const char *str, double color) {
		PrintGameStr(x, y, (char *)str, color);
	};
	draw["drawLine"] = [](double x0, double y0, double x1, double y1, double color) {
		DrawLine(x0, y0, x1, y1, color);
	};
}
