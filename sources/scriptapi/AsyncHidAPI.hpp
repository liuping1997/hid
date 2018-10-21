
#include <lua.h>
#include <lauxlib.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment (lib,"hid.lib")
#pragma comment (lib,"Setupapi.lib")
#pragma warning(disable:4244)
#endif

#ifndef TRUE
#define TRUE 1
#endif

#include "hidapi.h"
#include "AsyncHid.hpp"

// #include "luahidapi.h"
// #include "version.h"

namespace Lua
{
	static int async_init_hid(lua_State *L)
	{
		AsyncHid::Get().init();
		return 0;
	}

	static int async_open_hid(lua_State *L)
	{
		auto usVID = static_cast<const unsigned short>(lua_tointeger(L, -2));
		auto usPID = static_cast<const unsigned short>(lua_tointeger(L, -1));
		bool ret = AsyncHid::Get().open(usVID,usPID);
		lua_pushboolean(L, ret ? 1 : 0);
		return 1;
	}

	static int async_close_hid(lua_State *L)
	{
		AsyncHid::Get().close();
		return 0;
	}

	static int async_read_hid(lua_State *L)
	{
		auto len = static_cast<int>(lua_tointeger(L, -1));
		AsyncHid::ReadBuffer rbuf;
		AsyncHid::Get().read(rbuf);
		const char* buf = reinterpret_cast<const char*>(rbuf.data());
		lua_pushlstring(L, buf, len);
		return 1;
	}

	static int async_write_hid(lua_State *L)
	{
		size_t len = 0;
		auto data = reinterpret_cast<const unsigned char*>(lua_tolstring(L,-2, &len));
		AsyncHid::Buffer wbuf;
		memcpy_s(wbuf.data(), 128, data, len);
		AsyncHid::Get().write(wbuf);
		return 0;
	}

	LUALIB_API int async_hid_functions_bind(lua_State *L)
	{
		luaL_checkversion(L);
		static const luaL_Reg l[] = {
		{ "open" , async_open_hid},
		{ "close" , async_close_hid},
		{ "write" , async_write_hid},
		{ "read" , async_read_hid},
		{ NULL, NULL},
		};
		luaL_newlib(L, l);
		return 1;
	}

	void async_hid_register(lua_State *L)
	{
		luaL_requiref(L, "async_hidapi", async_hid_functions_bind, 0);
	}
}
