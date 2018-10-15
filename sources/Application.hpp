#pragma once 

#include "Exports.hpp"
#include "HidDevice.hpp"
#include "Logger.hpp"
#include "Lua.hpp"

using namespace std;

class Application
{
private:
	shared_ptr<CHidDevice> mHID = make_shared<CHidDevice>();

public:
	Application()
	{
	}

	static Application & instance()
	{
		static Application m_instance;
		return m_instance;
	}

	static void initialize()
	{
		// logger
		auto rotating_logger = spdlog::rotating_logger_mt("hid", "logs/hid.log", 1048576 * 5, 3);
		spdlog::flush_every(std::chrono::seconds(3));

		// lua
		struct lua_State *L = luaL_newstate();
		luaL_openlibs(L);
		luaL_loadfile(L, "./scritps/main.lua");

		// 获取函数，压入栈中  
		lua_getglobal(L, "load");
		// 调用函数，调用完成以后，会将返回值压入栈中，2表示参数个数，1表示返回结果个数。  
		if (lua_pcall(L, 0, 0, 0))// 调用出错  
		{
			const char *pErrorMsg = lua_tostring(L, -1);
			spdlog::info(lua_tostring(L, -1));
		}
	}

	shared_ptr<CHidDevice> getHID()
	{
		return mHID;
	}

	bool openDevice(unsigned short usVID, unsigned short usPID)
	{
		return mHID->open(usVID,usPID);
	}

	void closeDevice()
	{
		mHID->close();
	}



};

