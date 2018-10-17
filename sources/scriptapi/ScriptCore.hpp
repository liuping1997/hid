#pragma once

#include <lua.hpp>

namespace Lua
{
	void initialize();
	void close();
	void registerAPI(lua_State *L);
	void call(const char* func);
	void call(const char* func, double arg, int nresults);
	void call2Args(const char* func, int arg1, int arg2, int nresults);
};