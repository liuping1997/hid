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

		// ��ȡ������ѹ��ջ��  
		lua_getglobal(L, "load");
		// ���ú�������������Ժ󣬻Ὣ����ֵѹ��ջ�У�2��ʾ����������1��ʾ���ؽ��������  
		if (lua_pcall(L, 0, 0, 0))// ���ó���  
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

