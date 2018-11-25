#ifndef _API_H
#define _API_H

#include <lua.hpp>

extern lua_State *L;

void api_init();
void api_load_file(const char *filename);

/// Calls a function on the stack
void api_call_function();

#endif
