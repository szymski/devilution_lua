#ifndef _API_H
#define _API_H

#include <sol.hpp>

extern sol::state lua;

void api_init();
void api_load_file(const char *filename);

/// Calls a function on the stack
void api_call_function(const char *name);

#endif
