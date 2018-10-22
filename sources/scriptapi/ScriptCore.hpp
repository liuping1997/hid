#pragma once

#include <lua.hpp>

namespace Lua
{
	void initialize();
	void close();
	void eventLoop(double dt);
	void registerAPI(lua_State *L);
	void call(const char* func);
	void call(const char* func, double arg);
	void call2Args(const char* func, int arg1, int arg2);
	int lua_hid_read_int4(int id, int mask);
};