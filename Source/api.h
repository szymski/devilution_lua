#include <sol.hpp>
#include <unordered_set>
#include <unordered_map>

extern int api_last_monter_uid;
extern std::unordered_set<int> api_valid_monsters;
extern std::unordered_map<int, int> api_monster_id_to_uid;

extern sol::state lua;
extern std::unordered_map<std::string, std::unordered_map<std::string, sol::function>> hooks;

#ifndef _API_H
#define _API_H

#include "api_types.h"

void api_init();
void api_load_file(const char *filename);

void api_update_timers();

template <typename... Args>
sol::object api_call_hook_return(const char *name, Args... args)
{
	auto hooksCopy = hooks[name];

	sol::object obj;

	for (auto pair : hooksCopy) {
		obj = api_call_function<sol::object>(pair.second, args...);
		if (obj.valid())
			return obj;
	}

	return obj;
}

template <typename... Args>
void api_call_hook(const char *name, Args... args)
{
	auto hooksCopy = hooks[name];

	for (auto pair : hooksCopy) {
		api_call_function(pair.second, args...);
	}
}

template <typename T, typename... Args>
T api_call_function(sol::function f, Args... args)
{
	try {
		std::function<sol::object(Args...)> callback = f;
		return callback(args...);
	} catch (const sol::error &e) {
		std::cout << "Lua error: " << e.what() << std::endl;
	}
}

template <typename... Args>
void api_call_function(sol::function f, Args... args)
{
	try {
		std::function<void(Args...)> callback = f;
		callback(args...);
	} catch (const sol::error &e) {
		std::cout << "Lua error: " << e.what() << std::endl;
	}
}

void api_on_init_monster(int id);

void api_on_delete_monster(int id);

#endif
