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
		auto rotating_logger = spdlog::rotating_logger_mt("hid", "./logs/hid.log", 1048576 * 5, 3);
		spdlog::flush_every(std::chrono::seconds(1));
	
		Lua::initialize();
	}
	static void close()
	{
		Lua::close();
	}
};

