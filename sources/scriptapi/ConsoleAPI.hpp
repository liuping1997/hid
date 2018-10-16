#pragma once

#include <lua.hpp>
#include <windows.h>

using namespace std;

namespace Lua
{
	inline static HANDLE hConsole = nullptr;

	static int console_set_screen_buffer_size(lua_State *L)
	{
		if (hConsole == nullptr)
		{
			hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		}
		auto w = static_cast<short>(lua_tointeger(L, -2));
		auto h = static_cast<short>(lua_tointeger(L, -1));
		SetConsoleScreenBufferSize(hConsole, {w,h});
		return 0;
	}

	static int console_set_cursor_visible(lua_State *L)
	{
		if (hConsole == nullptr)
		{
			hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		}
		auto visible = lua_toboolean(L, -1) != 0 ?true:false;
		CONSOLE_CURSOR_INFO CursorInfo;
		GetConsoleCursorInfo(hConsole, &CursorInfo);
		CursorInfo.bVisible = visible; 
		SetConsoleCursorInfo(hConsole, &CursorInfo);
		return 0;
	}

	static int console_print(lua_State *L)
	{
		if (hConsole == nullptr)
		{
			hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		}
		auto x = static_cast<short>(lua_tointeger(L, -3));
		auto y = static_cast<short>(lua_tointeger(L, -2));
		auto text = lua_tostring(L, -1);
		SetConsoleCursorPosition(hConsole, {x,y});
		printf(text); 
		return 0;
	}

	static int console_close(lua_State *L)
	{
		CloseHandle(hConsole);
		return 0;
	}

	LUALIB_API int console_functions_bind(lua_State *L)
	{
		luaL_checkversion(L);
		static const luaL_Reg l[] = {
		{ "print" , console_print},
		{ "close" , console_close},
		{ "set_buffer_size" , console_set_screen_buffer_size},
		{ "set_cursor_visible" , console_set_cursor_visible},
		{ NULL, NULL},
		};
		luaL_newlib(L, l);
		return 1;
	}

	void console_register(lua_State *L)
	{
		luaL_requiref(L, "console", console_functions_bind, 0);
	}


};
