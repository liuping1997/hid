#include "Exports.hpp"
#include "Logger.hpp"
#include "Application.hpp"
#include "AsyncHid.hpp"
#include "ScriptAPI/ScriptCore.hpp"
#include <Windows.h>
#include <conio.h>
#include <filesystem>
#include <chrono>
#include  <boost/crc.hpp>

using namespace std;
using namespace chrono;

namespace fs = std::filesystem;
std::shared_ptr<Application> app;

using ushort = unsigned short;
using uchar = unsigned char;

extern "C"
{
	void usb_hid_int()
	{
		AsyncHid::Get().init();
	}

	bool usb_hid_open(ushort usVID, ushort usPID)
	{
		return AsyncHid::Get().open(usVID, usPID);
	}

	void usb_hid_close()
	{
		AsyncHid::Get().close();
	}

	void usb_hid_write(const uchar *wbuf,int len)
	{
		AsyncHid::Buffer buf;
		memcpy_s(buf.data(), buf.size(), wbuf, len);
		AsyncHid::Get().write(buf);
	}

	void usb_hid_read_all(uchar* buf, int len)
	{
		AsyncHid::Get().read(buf, len);
	}
	int usb_hid_read(int id, int mask)
	{
		return Lua::lua_hid_read_int4(id, mask);
	}
}

#ifdef _CONSOLE
int main (int argc, char **argv) 
{
	// This is "123456789" in ASCII
	unsigned char const data[] = { 0, 0, 2, 0, 2, 0, 0xb5 };
	std::size_t const data_len = sizeof(data) / sizeof(data[0]);

	// The expected CRC for the given data

	boost::uint16_t const expected = 0x1641;

	// Simulate CRC-CCITT
	boost::crc_basic<16> crc_ccitt22(0x1021, 0x0000, 0, true, true);
	crc_ccitt22.process_bytes(data, data_len);
	spdlog::info("{0:x}",crc_ccitt22.checksum());

	// Repeat with the optimal version (assuming a 16-bit type exists)

	boost::crc_optimal<16, 0x1021, 0, 0, true, true> crc_ccitt2;
	crc_ccitt2 = std::for_each(data, data + data_len, crc_ccitt2);
	spdlog::info("{0:x}",crc_ccitt2.checksum());
	
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
		app.reset(&Application::instance());
		app->initialize();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#endif // _CONSOLE
