#pragma once

#include "Proto.hpp"
#include "Lua.hpp"

using namespace std;

namespace Lua
{
	static int load_proto(lua_State *L)
	{
		vector<Proto::Item> items;
		Proto::Item item;
		luaL_checktype(L, 1, LUA_TTABLE);
		int index = lua_gettop(L); 
		lua_pushnil(L);  
		while (0 != lua_next(L, index))
		{
			string key = luaL_checkstring(L, -2);

			int it_idx = lua_gettop(L);
			lua_pushnil(L);
			while (lua_next(L, it_idx))
			{
				lua_rawgeti(L, -1, 1);
				item.id = luaL_checkinteger(L, -1);

				lua_rawgeti(L, -1, 2);
				item.offset= luaL_checkinteger(L, -1);

				lua_rawgeti(L, -1, 3);
				strcpy_s(item.type,sizeof(item.type), luaL_checkstring(L, -1));

				lua_rawgeti(L, -1, 4);
				strcpy_s(item.name,sizeof(item.name), luaL_checkstring(L, -1));

				lua_pop(L, 1);
				items.push_back(item);
			}
			lua_pop(L, 1); 
		}
		Proto::load(items);
		return 0;
	}

	LUAMOD_API int bind(lua_State *L) 
	{
		luaL_checkversion(L);
		luaL_Reg l[] = {
		{ "load_proto" , load_proto},
		{ NULL, NULL },
		};
		luaL_setfuncs(L, l, 0);
	}
};

