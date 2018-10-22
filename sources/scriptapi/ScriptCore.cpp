#pragma once

#include "ScriptCore.hpp"
#include "Logger.hpp"
#include "Application.hpp"
#include "ApplicationAPI.hpp"
#include "ConsoleAPI.hpp"
#include "HidAPI.hpp"
#include "AsyncHidAPI.hpp"
#include "UtilsAPI.hpp"
#include "lfs.h"
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

namespace Lua
{
	inline static lua_State* L = nullptr;
	inline static bool lua_actived = false;

	void initialize()
	{
		L = luaL_newstate();
		luaL_openlibs(L);
		luaopen_lfs(L);
		registerAPI(L);

		// load main.lua
		array<string,4> paths = {
			"./scripts/bootstrap.lua",
			"./bootstrap.lua",
			"../scripts/bootstrap.lua",
			"../../scripts/bootstrap.lua"
		};
		auto path = paths[0];
		for (auto p : paths)
		{
			if (fs::exists(p))
				path = p;
		}
		spdlog::info("lua path:{0}", fs::absolute(path).generic_string());
		if (luaL_loadfile(L, path.c_str()) != LUA_OK)
		{
			lua_actived = false;
			spdlog::info(lua_tostring(L, -1));
			return;
		}
		if (lua_pcall(L, 0, 0, 0) != LUA_OK)
		{
			spdlog::info(lua_tostring(L, -1));
			return;
		}

		lua_actived = true;
		if (lua_getglobal(L, "init") == LUA_TFUNCTION)
		{
			if (lua_pcall(L, 0, 0, 0) != LUA_OK)
			{
				spdlog::info(lua_tostring(L, -1));
			}
		}
	}
	
	void close()
	{
		lua_close(L);
	}

	void registerAPI(lua_State* l)
	{
		application_register(l);
		console_register(l);
		hid_register(l);
		async_hid_register(l);
		utils_register(l);
		Lua::L = l;
	}

	void eventLoop(double dt)
	{
		call("event_loop", dt);
	}

	void call(const char* func)
	{
		if (!lua_actived)
			return;
		int ret = lua_getglobal(L, func);
		if (lua_pcall(L, 0, 0, 0))
		{
			spdlog::info(lua_tostring(L, -1));
		}
	}

	void call(const char* func, double arg)
	{
		if (!lua_actived)
			return;
		int ret = lua_getglobal(L, func);
		lua_pushnumber(L, arg);

		if (lua_pcall(L, 1, 0, 0))
		{
			spdlog::info(lua_tostring(L, -1));
		}
	}


	void call2Args(const char* func, int arg1, int arg2)
	{
		if (!lua_actived)
			return;
		int ret = lua_getglobal(L, func);
		lua_pushinteger(L, arg1);
		lua_pushinteger(L, arg2);
		if (lua_pcall(L, 2, 0, 0))
		{
			spdlog::info(lua_tostring(L, -1));
		}
	}

	int lua_hid_read_int4(int id, int mask)
	{
		if (!lua_actived)
			return -1;
		int ret = lua_getglobal(L, "hid_read_by_id");
		lua_pushinteger(L, id);
		lua_pushinteger(L, mask);

		if (lua_pcall(L, 2, 1, 0) != LUA_OK)
		{
			spdlog::error(lua_tostring(L, -1));
			return -1;
		}
		return static_cast<int>(lua_tointeger(L, -1));
	}
};