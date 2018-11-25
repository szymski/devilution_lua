#include "api.h"
#include "../types.h"
#include "api_types.h"

#include <chrono>
#include <fcntl.h>

sol::state lua;

std::vector<api_Timer> timer_list;
std::vector<api_Timer> timers_to_add;

void api_register_functions();
void api_init_console();
void api_init_timer();

void api_init()
{
	api_init_console();
	api_init_timer();

	printf("Initializing Lua API\n");

	lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table, sol::lib::os);

	api_register_functions();

	api_load_file("init.lua");
}

void api_init_console()
{
	freopen("//./NUL", "w", stdout);
	setvbuf(stdout, NULL, _IONBF, 0);

	AllocConsole();
	void *hStd = GetStdHandle(STD_OUTPUT_HANDLE);
	int fdStd = _open_osfhandle((intptr_t)hStd, _O_TEXT);
	_dup2(fdStd, fileno(stdout));
	SetStdHandle(STD_OUTPUT_HANDLE, (HANDLE)_get_osfhandle(fileno(stdout)));
	_close(fdStd);
}

void api_load_file(const char *filename)
{
	try {
		auto result = lua.safe_script_file(filename);
	} catch (const sol::error &e) {
		std::cout << "Lua error: " << e.what() << std::endl;
	}
}

void api_init_timer()
{
}

void api_update_timers()
{
	auto now = ApiClock::now();

	auto it = timer_list.begin();
	while (it != timer_list.end()) {
		if (now >= it->nextTick) {
			api_call_function(it->callback);
			it = timer_list.erase(it);
		} else
			++it;
	}

	for (auto timer : timers_to_add)
		timer_list.push_back(timer);

	timers_to_add.clear();
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

void api_call_function(sol::function f)
{
	try {
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

	// Timer library

	auto timer = lua.create_table();
	lua["timer"] = timer;

	timer["simple"] = [](double delay, sol::function callback) {
		api_Timer timer{ ApiClock::now() + std::chrono::milliseconds((long)(delay * 1000.0)), callback };
		timers_to_add.push_back(timer);
	};

	// Player library

	auto playerType = lua.new_usertype<api_PlayerStruct>("Player",
	    "getName", &api_PlayerStruct::getName,

	    "getHP", &api_PlayerStruct::getHP,
	    "setHP", &api_PlayerStruct::setHP,

	    "getGold", &api_PlayerStruct::getGold,
	    "setGold", &api_PlayerStruct::setGold,

	    "kill", &api_PlayerStruct::kill);

	auto player = lua.create_table();
	lua["player"] = player;

	player["getLocalPlayer"] = []() {
		api_PlayerStruct ply{ myplr, plr[myplr] };
		return ply;
	};

	// Mouse library

	auto mouse = lua.create_table();
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
