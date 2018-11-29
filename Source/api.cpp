#include "api.h"
#include "../types.h"
#include "api_types.h"

#include <chrono>
#include <fcntl.h>
#include <map>

sol::state lua;

std::vector<api_Timer> timer_list;
std::vector<api_Timer> timers_to_add;
std::chrono::time_point<ApiClock> start_time;

std::unordered_map<std::string, std::unordered_map<std::string, sol::function>> hooks;

void api_register_functions();
void api_init_console();
void api_init_timer();

void api_init()
{
	api_init_console();
	api_init_timer();

	printf("Initializing Lua API\n");

	lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::jit, sol::lib::math, sol::lib::table, sol::lib::os, sol::lib::package, sol::lib::utf8);

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
	start_time = ApiClock::now();
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

std::unordered_map<int, sol::object> api_players;

sol::object api_get_player(int id)
{
	auto it = api_players.find(id);
	if (it != api_players.end())
		return it->second;

	auto p = sol::make_object(lua, new api_PlayerStruct(id, plr[id]));
	api_players[id] = p;

	return p;
}

std::unordered_map<int, sol::object> api_monsters;

void api_on_init_monster(int id)
{
	auto apiMonster = new api_MonsterStruct(id, monster[id]);
	auto monsterObj = sol::make_object(lua, apiMonster);

	api_monsters[id] = monsterObj;

	api_call_hook("InitMonster", monsterObj);
}

sol::object api_get_monster(int id)
{
	auto it = api_monsters.find(id);
	if (it != api_monsters.end())
		return it->second;

	throw std::runtime_error("Monster not registered before using.");
}

void api_on_delete_monster(int id)
{
	auto it = api_monsters.find(id);
	if (it != api_monsters.end()) {
		auto obj = (*it).second;
		auto monster = obj.as<api_MonsterStruct *>();
		delete monster;
		// TODO: Call to invalid pointer.
		api_monsters.erase(it);
	}
}

#define REGISTER_ENUM(NAME) \
	lua[#NAME] = (int)NAME

void api_register_functions()
{
	lua["print"] = [](sol::variadic_args args) {
		for (auto it = args.begin(); it != args.end(); it++) {
			std::cout << lua["tostring"].call<std::string>((*it));
			if (it + 1 != args.end())
				std::cout << ", ";
		}

		std::cout << std::endl;
	};

	// Hook library

	auto hook = lua.create_table();
	lua["hook"] = hook;

	hook["add"] = [](const char *hookname, const char *identifier, sol::function callback) {
		hooks[hookname][identifier] = callback;
	};

	hook["remove"] = [](const char *hookname, const char *identifier) {
		hooks[hookname].erase(identifier);
	};

	// Timer library

	auto timer = lua.create_table();
	lua["timer"] = timer;

	timer["getTime"] = []() {
		auto time = ApiClock::now() - start_time;
		return std::chrono::duration_cast<std::chrono::milliseconds>(time).count() / 1000.0;
	};

	timer["simple"] = [](double delay, sol::function callback) {
		api_Timer timer{ ApiClock::now() + std::chrono::milliseconds((long)(delay * 1000.0)), callback };
		timers_to_add.push_back(timer);
	};

	// Player library

	REGISTER_ENUM(PC_WARRIOR);
	REGISTER_ENUM(PC_ROGUE);
	REGISTER_ENUM(PC_SORCERER);

	auto playerType = lua.new_usertype<api_PlayerStruct>("Player",
	    "getName", &api_PlayerStruct::getName,
	    "getClass", &api_PlayerStruct::getClass,

	    "getHP", &api_PlayerStruct::getHP,
	    "setHP", &api_PlayerStruct::setHP,
	    "getMaxHP", &api_PlayerStruct::getMaxHP,

	    "getMana", &api_PlayerStruct::getMana,
	    "setMana", &api_PlayerStruct::setMana,
	    "getMaxMana", &api_PlayerStruct::getMaxMana,

	    "getGold", &api_PlayerStruct::getGold,
	    "setGold", &api_PlayerStruct::setGold,

	    "getLevel", &api_PlayerStruct::getLevel,

	    "setPos", &api_PlayerStruct::setPos,
	    "getPos", &api_PlayerStruct::getPos,

	    "kill", &api_PlayerStruct::kill);

	auto player = lua.create_table();
	lua["player"] = player;

	player["getLocalPlayer"] = []() {
		return api_get_player(myplr);
	};

	// Monster library

	auto monsterType = lua.new_usertype<api_MonsterStruct>("Monster",
	    "getName", &api_MonsterStruct::getName,

	    "getHP", &api_MonsterStruct::getHP,
	    "setHP", &api_MonsterStruct::setHP,

	    "getLevel", &api_MonsterStruct::getLevel,
	    "setLevel", &api_MonsterStruct::setLevel,

	    "getPos", &api_MonsterStruct::getPos,
	    "setPos", &api_MonsterStruct::setPos,

	    "kill", &api_MonsterStruct::kill);

	auto monsterTbl = lua.create_table();
	lua["monster"] = monsterTbl;

	monsterTbl["spawn"] = [](double x, double y, double dir, double type, bool inMap) -> sol::object {
		int i = AddMonster(x, y, dir, type, inMap);
		if (i > 0)
			return api_get_monster(i);

		return sol::nil;
	};

	monsterTbl["getAll"] = []() {
		std::vector<sol::object> result;

		for (auto id : api_monsters)
			result.push_back(api_get_monster(id.first));

		return sol::as_table(result);
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

	draw["printGameStr"] = [](double x, double y, const char *str, sol::optional<double> color) {
		if (x >= 0 && y >= 0)
			PrintGameStr(x, y, (char *)str, color.value_or(COL_WHITE));
	};

	draw["getTextWidth"] = [](const char* str) {
		int width = 0;
		for (const char *c = str; *c != 0; c++)
			width += fontkern[fontframe[fontidx[*c]]] + 1;
		return width;
	};

	draw["drawLine"] = [](double x0, double y0, double x1, double y1, double color) {
		DrawLine(x0, y0, x1, y1, color);
	};

	draw["drawRect"] = [](double x_, double y_, double w, double h, double color) {
		for (int y = max(0, y_); y < min(480, y_ + h); y++) {
			for (int x = max(0, x_); x < min(640, x_ + w); x++) {
				gpBuffer->row[y].pixels[x] = color;
			}
		}
	};

	draw["drawRectTransparent"] = [](double x_, double y_, double w, double h, double color) {
		for (int y = max(0, y_); y < min(480, y_ + h); y++) {
			for (int x = max(0, x_) + y % 2; x < min(640, x_ + w); x += 2) {
				gpBuffer->row[y].pixels[x] = color;
			}
		}
	};

	// Item library

	REGISTER_ENUM(ITYPE_0E);
	REGISTER_ENUM(ITYPE_AMULET);
	REGISTER_ENUM(ITYPE_AXE);
	REGISTER_ENUM(ITYPE_BOW);
	REGISTER_ENUM(ITYPE_GOLD);
	REGISTER_ENUM(ITYPE_HARMOR);
	REGISTER_ENUM(ITYPE_HELM);
	REGISTER_ENUM(ITYPE_LARMOR);
	REGISTER_ENUM(ITYPE_MACE);
	REGISTER_ENUM(ITYPE_MARMOR);
	REGISTER_ENUM(ITYPE_MISC);
	REGISTER_ENUM(ITYPE_RING);
	REGISTER_ENUM(ITYPE_SHIELD);
	REGISTER_ENUM(ITYPE_STAFF);
	REGISTER_ENUM(ITYPE_SWORD);
	REGISTER_ENUM(ITYPE_API_CUSTOM);

	REGISTER_ENUM(IMISC_NONE);
	REGISTER_ENUM(IMISC_USEFIRST);
	REGISTER_ENUM(IMISC_FULLHEAL);
	REGISTER_ENUM(IMISC_HEAL);
	REGISTER_ENUM(IMISC_OLDHEAL);
	REGISTER_ENUM(IMISC_DEADHEAL);
	REGISTER_ENUM(IMISC_MANA);
	REGISTER_ENUM(IMISC_FULLMANA);
	REGISTER_ENUM(IMISC_POTEXP);
	REGISTER_ENUM(IMISC_POTFORG);
	REGISTER_ENUM(IMISC_ELIXSTR);
	REGISTER_ENUM(IMISC_ELIXMAG);
	REGISTER_ENUM(IMISC_ELIXDEX);
	REGISTER_ENUM(IMISC_ELIXVIT);
	REGISTER_ENUM(IMISC_ELIXWEAK);
	REGISTER_ENUM(IMISC_ELIXDIS);
	REGISTER_ENUM(IMISC_ELIXCLUM);
	REGISTER_ENUM(IMISC_ELIXSICK);
	REGISTER_ENUM(IMISC_REJUV);
	REGISTER_ENUM(IMISC_FULLREJUV);
	REGISTER_ENUM(IMISC_USELAST);
	REGISTER_ENUM(IMISC_SCROLL);
	REGISTER_ENUM(IMISC_SCROLLT);
	REGISTER_ENUM(IMISC_STAFF);
	REGISTER_ENUM(IMISC_BOOK);
	REGISTER_ENUM(IMISC_RING);
	REGISTER_ENUM(IMISC_AMULET);
	REGISTER_ENUM(IMISC_UNIQUE);
	REGISTER_ENUM(IMISC_HEAL_1C);
	REGISTER_ENUM(IMISC_OILFIRST);
	REGISTER_ENUM(IMISC_OILOF);
	REGISTER_ENUM(IMISC_OILACC);
	REGISTER_ENUM(IMISC_OILMAST);
	REGISTER_ENUM(IMISC_OILSHARP);
	REGISTER_ENUM(IMISC_OILDEATH);
	REGISTER_ENUM(IMISC_OILSKILL);
	REGISTER_ENUM(IMISC_OILBSMTH);
	REGISTER_ENUM(IMISC_OILFORT);
	REGISTER_ENUM(IMISC_OILPERM);
	REGISTER_ENUM(IMISC_OILHARD);
	REGISTER_ENUM(IMISC_OILIMP);
	REGISTER_ENUM(IMISC_OILLAST);
	REGISTER_ENUM(IMISC_MAPOFDOOM);
	REGISTER_ENUM(IMISC_EAR);
	REGISTER_ENUM(IMISC_SPECELIX);
	REGISTER_ENUM(IMISC_API_CUSTOM);

	auto item = lua.create_table();
	lua["item"] = item;

	item["dropRandomItem"] = [](double x, double y, sol::optional<bool> onlyGood) {
		CreateRndItem(x, y, onlyGood.value_or(false), false, 0);
	};

	item["dropRandomItemType"] = [](double x, double y, double type, sol::optional<double> miscId, double level, sol::optional<bool> onlyGood) {
		if (numitems < MAXITEMS) {
			int ii = itemavail[0];
			GetSuperItemSpace(x, y, ii);
			itemactive[numitems] = ii;
			itemavail[0] = itemavail[MAXITEMS - numitems - 1];

			SetupAllItems(ii, RndTypeItems(type, miscId.value_or(-1)), GetRndSeed(), level, 1, onlyGood.value_or(false), 0, 0);

			++numitems;
		}
	};
}
