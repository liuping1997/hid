#include "Exports.hpp"
#include "Logger.hpp"
#include "Application.hpp"
#include "AsyncHid.hpp"
#include "ScriptAPI/ScriptCore.hpp"
#include <Windows.h>
#include <conio.h>
#include <filesystem>
#include <chrono>
#include <boost/crc.hpp>

using namespace std;
using namespace chrono;

namespace fs = std::filesystem;
std::shared_ptr<Application> app;

using ushort = unsigned short;
using uchar = unsigned char;

extern "C"
{
	void hidapi_int()
	{
		AsyncHid::Get().init();
	}

	bool hidapi_open(ushort usVID, ushort usPID)
	{
		return AsyncHid::Get().open(usVID, usPID);
	}

	void hidapi_close()
	{
		AsyncHid::Get().close();
	}

	void hidapi_write(const uchar *wbuf, int len)
	{
		AsyncHid::Buffer buf;
		buf[0] = 0;
		len = std::clamp(len, 0, (int)buf.size() - 1);
		memcpy_s(buf.data() + 1, buf.size() - 1, wbuf, len);
		AsyncHid::Get().write(buf);
	}

	void hidapi_write_crc16(const uchar *wbuf,int len)
	{
		AsyncHid::Buffer buf;
		buf[0] = 0;
		if (len > (buf.size() - 1))
			len = buf.size() - 1;
		memcpy_s(buf.data() + 1, buf.size() - 1, wbuf, len);
		boost::crc_optimal<16, 0x1021, 0, 0, true, true> crc_ccitt_kermit;
		crc_ccitt_kermit = std::for_each(wbuf, wbuf + len, crc_ccitt_kermit);
		ushort sum = crc_ccitt_kermit.checksum();
		buf[len + 1] = sum & 0x00FF;
		buf[len + 2] = sum >> 8;
		buf[buf.size() - 1] = len + 2;
		AsyncHid::Get().write(buf);
	}

	void hidapi_read(uchar* buf, int len)
	{
		AsyncHid::Get().read(buf, len);
	}
}

#ifdef _CONSOLE
int main (int argc, char **argv) 
{
	fs::path apppath = argv[0];
	fs::current_path(apppath.remove_filename());
	spdlog::info("current apppath:{0}", fs::current_path().generic_string());
	app.reset(&Application::instance());
	app->initialize();
	spdlog::info("\n[1] open\n[2] close\n[3] read\n[4] write\n[5] print\n[c] clear & help\n[q] quit\n");
	time_point<system_clock> start = system_clock::now();
	while (true)
	{
		auto now = system_clock::now();
		duration<double> diff = now - start;
		start = now;
		Lua::eventLoop(diff.count());
		if (_kbhit())
		{
			char c = _getch();
			if (c == 'q')
			{
				break;
			}
			else if (c == 'c' || c == 'h')
			{
				system("cls");
				spdlog::info("\n[1] open\n[2] close\n[3] read\n[4] write\n[5] print\n[c] clear & help\n[q] quit\n");
			}
			else if (c == '1')
			{
				spdlog::info("test hid open");
				Lua::call("open_hid");
			}
			else if (c == '2')
			{
				spdlog::info("test hid close");
				Lua::call("close_hid");
			}
			else if (c == '3')
			{
				spdlog::info("test hid read");
				Lua::call("test_read_hid");
			}
			else if (c == '4')
			{
				spdlog::info("test hid write");
				Lua::call("test_write_hid");
			}
			else if (c == '5')
			{
				Lua::call("print_hid");
			}
			else if (c == 't')
			{
				Lua::call("test_func");
			}
		}
	}
	Application::close();
	std::exit(0);
	return -1;
}

#else
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		spdlog::info("current apppath:{0}", fs::current_path().generic_string());
		app.reset(&Application::instance());
		app->initialize();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		Application::close();
		break;
	}
	return TRUE;
}

#endif // _CONSOLE
