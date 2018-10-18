#pragma once 

#include "Exports.hpp"
#include "HidDevice.hpp"
#include "Logger.hpp"
#include "ScriptAPI/ScriptCore.hpp"
#include <filesystem>
#include <lua.hpp>
#include <chrono>

using namespace std;
using namespace chrono;

class Application
{
	using uchar = unsigned char;

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
		std::filesystem::create_directory("./logs/");
		auto rotating_logger = spdlog::rotating_logger_mt("hid", "./logs/hid.log", 1048576 * 5, 3);
		spdlog::flush_every(std::chrono::seconds(3));
	
		Lua::initialize();
	}

	static void close()
	{
		Lua::close();
		Application::instance().getHID()->quit();
	}

	shared_ptr<CHidDevice> getHID()
	{
		return mHID;
	}

	bool openDevice(unsigned short usVID, unsigned short usPID)
	{
		bool ret = mHID->open(usVID,usPID);
		ret ? spdlog::info("open hid success"): spdlog::error("open hid failure");
		return ret;
	}

	void closeDevice()
	{
		spdlog::info("close hid success");
		mHID->close();
	}

	uchar* read(uchar cmd, int len)
	{
		static CHidDevice::Buffer buf;
		len = std::clamp(len, 0, 0x26);
		buf[6] = cmd;
		buf[buf.size() - 1] = len;
		mHID->read(buf);
		return buf.data();
	}
	
	void write(const uchar *cmd,int len)
	{
		auto now = system_clock::now();
		CHidDevice::Buffer buf;
		memcpy_s(buf.data(), len, cmd, len);
		buf[buf.size() - 1] = len;
		mHID->write(std::move(buf));
		auto start = system_clock::now();
		duration<double> diff = start - now;
		spdlog::info("write:", diff.count());
	}
};

