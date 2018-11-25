#ifndef _API_H
#define _API_H

#include <sol.hpp>

extern sol::state lua;

void api_init();
void api_load_file(const char *filename);

void api_update_timers();

void api_call_function(const char *name);
void api_call_function(sol::function f);

#endif
