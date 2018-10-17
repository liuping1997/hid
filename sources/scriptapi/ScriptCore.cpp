#pragma once

#include "ScriptCore.hpp"
#include "Logger.hpp"
#include "Application.hpp"
#include "ApplicationAPI.hpp"
#include "ConsoleAPI.hpp"
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
		registerAPI(L);

		array<string,4> paths = {
			"./scripts/main.lua",
			"./main.lua",
			"../scripts/main.lua",
			"../../scripts/main.lua"
		};
		string path = paths[0];
		for (auto p : paths)
		{
			if (fs::exists(p))
				path = p;
		}
		spdlog::info(fs::absolute(path).string());
		if (luaL_loadfile(L, path.c_str()))
		{
			lua_actived = false;
			spdlog::info(lua_tostring(L, -1));
			return;
		}
		lua_actived = true;
		int ret = lua_getglobal(L, "init");
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
		console_register(l);
		Lua::L = l;
	}

	void call(const char* func)
	{
		if (!lua_actived)
			return;
		try
		{
			int ret = lua_getglobal(L, func);
			if (lua_pcall(L, 0, 0, 0))
			{
				spdlog::info(lua_tostring(L, -1));
				throw("lua error");
			}
		}
		catch (...)
		{
			lua_actived = false;
			spdlog::error("lua runtime error.", __FUNCTION__);
		}
	}

	void call(const char* func, double arg, int nresults)
	{
		if (!lua_actived)
			return;
		try
		{
			int ret = lua_getglobal(L, func);
			lua_pushnumber(L, arg);

			if (lua_pcall(L, 1, 0, 0))
			{
				spdlog::info(lua_tostring(L, -1));
				throw("lua error");
			}
		}
		catch (...)
		{
			lua_actived = false;
			spdlog::error("lua runtime error.", __FUNCTION__);
		}
	}

	void call2Args(const char* func, int arg1, int arg2, int nresults)
	{
		if (!lua_actived)
			return;
		try
		{
			int ret = lua_getglobal(L, func);
			lua_pushinteger(L, arg1);
			lua_pushinteger(L, arg2);
			if (lua_pcall(L, 2, nresults, 0))
			{
				spdlog::info(lua_tostring(L, -1));
				throw("lua error");
			}
		}
		catch (...)
		{
			lua_actived = false;
			spdlog::error("lua runtime error.",__FUNCTION__);
		}
	}

};