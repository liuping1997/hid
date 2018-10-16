#pragma once

#include "ScriptCore.hpp"
#include "Logger.hpp"
#include "Application.hpp"
#include "ApplicationAPI.hpp"

namespace Lua
{
	inline static lua_State* L = nullptr;

	void initialize()
	{
		L = luaL_newstate();
		luaL_openlibs(L);
		registerAPI(L);
		if (luaL_loadfile(L, "../scripts/main.lua"))
		{
			spdlog::info(lua_tostring(L, -1));
			return;
		}
		int ret = lua_pcall(L, 0, 0, 0);
		ret = lua_getglobal(L, "load");
		if (lua_pcall(L, 0, 0, 0))
		{
			spdlog::info(lua_tostring(L, -1));
		}
	}
	
	void close()
	{
		lua_close(L);
	}

	void registerAPI(lua_State* l)
	{
		application_register(l);
		Lua::L = l;
	}

	void call(const char* func,int nresults)
	{
		int ret = lua_getglobal(L, func);
		if (lua_pcall(L, 0, nresults, 0))
		{
			spdlog::info(lua_tointeger(L, -1));
		}
	}

	void call(const char* func)
	{
		call(func, 0);
	}

	void call(const char* func, int arg1, int arg2, int nresults)
	{
		int ret = lua_getglobal(L, func);
		lua_pushinteger(L, arg1);
		lua_pushinteger(L, arg2);
		if (lua_pcall(L, 2, nresults, 0))
		{
			spdlog::info(lua_tostring(L, -1));
		}
	}

};