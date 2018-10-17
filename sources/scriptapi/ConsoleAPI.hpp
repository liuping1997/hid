#pragma once

#include <lua.hpp>
#include <windows.h>

using namespace std;

namespace Lua
{
	inline static HANDLE handle = nullptr;

	static int console_set_cp(lua_State *L)
	{
		auto cp = static_cast<int>(lua_tointeger(L, -2));
		auto fontsize = static_cast<short>(lua_tointeger(L, -1));
		SetConsoleOutputCP(cp);
		CONSOLE_FONT_INFOEX info = { 0 }; 
		info.cbSize = sizeof(info);
		info.dwFontSize.Y = fontsize; 
		info.FontWeight = FW_NORMAL;
		wcscpy_s(info.FaceName, LF_FACESIZE,L"Consolas");
		SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, &info);
		return 0;
	}

	static int console_set_window_size(lua_State *L)
	{
		if (handle == nullptr)
			handle	= GetStdHandle(STD_OUTPUT_HANDLE);
		auto w = static_cast<short>(lua_tointeger(L, -2));
		auto h = static_cast<short>(lua_tointeger(L, -1));
		CONSOLE_SCREEN_BUFFER_INFOEX info;
		info.cbSize = sizeof(info);
		bool ret = GetConsoleScreenBufferInfoEx(handle, &info);
		info.dwSize = { w, h };
		int _w = (w - (info.srWindow.Right - info.srWindow.Left)) / 2;
		int _h = (h - (info.srWindow.Bottom - info.srWindow.Top)) / 2;
		info.srWindow.Left += _w;
		info.srWindow.Top += _h;
		info.srWindow.Right = info.srWindow.Left + w;
		info.srWindow.Bottom = info.srWindow.Top + h;
		ret = SetConsoleScreenBufferInfoEx(handle, &info);
		return 0;
	}

	static int console_set_screen_buffer_size(lua_State *L)
	{
		if (handle == nullptr)
			handle = GetStdHandle(STD_OUTPUT_HANDLE);
		auto w = static_cast<short>(lua_tointeger(L, -2));
		auto h = static_cast<short>(lua_tointeger(L, -1));
		SetConsoleScreenBufferSize(handle, {w,h});
		return 0;
	}

	static int console_set_cursor_visible(lua_State *L)
	{
		if (handle == nullptr)
		{
			handle = GetStdHandle(STD_OUTPUT_HANDLE);
		}
		auto visible = lua_toboolean(L, -1) != 0 ?true:false;
		CONSOLE_CURSOR_INFO CursorInfo;
		GetConsoleCursorInfo(handle, &CursorInfo);
		CursorInfo.bVisible = visible; 
		SetConsoleCursorInfo(handle, &CursorInfo);
		return 0;
	}

	static int console_print(lua_State *L)
	{
		if (handle == nullptr)
			handle = GetStdHandle(STD_OUTPUT_HANDLE);
		auto x = static_cast<short>(lua_tointeger(L, -3));
		auto y = static_cast<short>(lua_tointeger(L, -2));
		auto text = lua_tostring(L, -1);
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(handle, &info);
		SetConsoleCursorPosition(handle, {x,y});
		printf(text); 
		SetConsoleCursorPosition(handle, info.dwCursorPosition);
		return 0;
	}

	static int console_close(lua_State *L)
	{
		CloseHandle(handle);
		return 0;
	}

	LUALIB_API int console_functions_bind(lua_State *L)
	{
		luaL_checkversion(L);
		static const luaL_Reg l[] = {
		{ "print" , console_print},
		{ "close" , console_close},
		{ "set_cp" , console_set_cp},
		{ "set_buffer_size" , console_set_screen_buffer_size},
		{ "set_window_size" , console_set_window_size},
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
