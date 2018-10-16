#pragma once

#include <lua.hpp>

namespace Lua
{
	void initialize();
	void close();
	void registerAPI(lua_State *L);
	void call(const char* func);
	void call(const char* func, int nresults);
	void call(const char* func, int arg1, int arg2, int nresults);
};