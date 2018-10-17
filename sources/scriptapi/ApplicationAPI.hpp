#pragma once

#include "Proto.hpp"
#include "Application.hpp"
#include <lua.hpp>

using namespace std;

namespace Lua
{
	/*
	static int load_proto(lua_State *L)
	{
		vector<Proto::Item> items;
		Proto::Item item;
		luaL_checktype(L, 1, LUA_TTABLE);
		int index = lua_gettop(L); 
		lua_pushnil(L);  
		while (0 != lua_next(L, -2))
		{
			// note :https://blog.csdn.net/Andy_93/article/details/79489728 遍历table不要对key进行lua_tostring
			lua_rawgeti(L, -1, 1);
			item.id = lua_tointeger(L, -1);
			lua_pop(L, 1);

			lua_rawgeti(L, -1, 2);
			item.offset = lua_tointeger(L, -1);
			lua_pop(L, 1);

			lua_rawgeti(L, -1, 3);
			strcpy_s(item.type, sizeof(item.type), lua_tostring(L, -1));
			lua_pop(L, 1);

			lua_rawgeti(L, -1, 4);
			strcpy_s(item.name, sizeof(item.name), lua_tostring(L, -1));
			lua_pop(L, 1);

			lua_pop(L, 1);
			luaL_checktype(L, -2, LUA_TTABLE);
			items.push_back(item);
		}
		Proto::load(items);
		return 0;
	}
	*/

	static int open_hid(lua_State *L)
	{
		auto usVID = static_cast<const unsigned short>(lua_tointeger(L, -2));
		auto usPID = static_cast<const unsigned short>(lua_tointeger(L, -1));
		return Application::instance().openDevice(usVID,usPID);
	}

	static int close_hid(lua_State *L)
	{
		Application::instance().closeDevice();
		return 0;
	}

	static int reset_hid(lua_State *L)
	{
		Application::instance().getHID().reset();
		return 0;
	}

	static int read_hid(lua_State *L)
	{
		auto cmd = static_cast<unsigned char>(lua_tointeger(L, -2));
		auto len = static_cast<int>(lua_tointeger(L, -1));
		const char* buf = reinterpret_cast<const char*>(Application::instance().read(cmd,len));
		lua_pushlstring(L, buf, len);
		return 1;
	}

	static int write_hid(lua_State *L)
	{
		size_t rellen = 0;
		auto data = reinterpret_cast<const unsigned char*>(lua_tolstring(L,-2, &rellen));
		auto len = static_cast<int>(lua_tointeger(L, -1));
		Application::instance().write(data, len);
		return 0;
	}

	LUALIB_API int applicaion_functions_bind(lua_State *L)
	{
		luaL_checkversion(L);
		static const luaL_Reg l[] = {
		{ "open" , open_hid},
		{ "close" , close_hid},
		{ "reset" , reset_hid},
		{ "write" , write_hid},
		{ "read" , read_hid},
		{ NULL, NULL},
		};
		luaL_newlib(L, l);
		return 1;
	}

	void application_register(lua_State *L)
	{
		luaL_requiref(L, "app", applicaion_functions_bind, 0);
	}
};
