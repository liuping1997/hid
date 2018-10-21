#pragma once

#include <lua.hpp>
#include <iostream>
#include <algorithm>
#include <boost/crc.hpp>

using namespace std;

namespace Lua
{
	using ushort = unsigned short;
	using uchar = unsigned char;

	int crc16_kermit(lua_State *L)
	{
		/*
		name    polynomial  initial val  reverse byte ? reverse result ? swap result ?
			CCITT         1021         ffff             no               no            no
			XModem        1021         0000             no               no            no
			Kermit        1021         0000            yes              yes           yes
			CCITT 1D0F    1021         1d0f             no               no            no
			IBM           8005         0000            yes              yes            no
		*/
		size_t len = 0;
		auto data = reinterpret_cast<const uchar*>(lua_tolstring(L,-1, &len));
		boost::crc_optimal<16, 0x1021, 0, 0, true, true> crc_ccitt_kermit;
		crc_ccitt_kermit = std::for_each(data, data + len, crc_ccitt_kermit);

		ushort sum = crc_ccitt_kermit.checksum();
		char* bytes = reinterpret_cast<char*>(&sum);
		std::swap(bytes[0], bytes[1]);
		lua_pushlstring(L, bytes, 2);
		return 1;
	}

	LUALIB_API int utils_functions_bind(lua_State *L)
	{
		luaL_checkversion(L);
		static const luaL_Reg l[] = {
		{ "crc16_kermit" , crc16_kermit},
		{ NULL, NULL},
		};
		luaL_newlib(L, l);
		return 1;
	}

	void utils_register(lua_State *L)
	{
		luaL_requiref(L, "utils", utils_functions_bind, 0);
	}
};
