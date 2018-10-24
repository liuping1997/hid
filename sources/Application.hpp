#pragma once 

#include "Exports.hpp"
#include "AsyncHid.hpp"
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
	using ushort = unsigned short;
	using uint = unsigned int;

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

		try
		{
			//spdlog::set_level(spdlog::level::info); // Set global log level to info
			//auto logger = spdlog::rotating_logger_mt("hid", "./logs/hid.log", 1048576 * 5, 3);
			//spdlog::set_default_logger(logger);
			//spdlog::flush_every(std::chrono::seconds(3));
		}
		catch (const spdlog::spdlog_ex &ex)
		{
			std::printf("Log initialization failed: %s\n", ex.what());
		}
		Lua::initialize();
	}
	static void close()
	{
		Lua::close();
	}
};

