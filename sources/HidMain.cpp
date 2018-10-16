#include "Exports.hpp"
#include "HidDevice.hpp"
#include "Logger.hpp"
#include "Application.hpp"
#include "ScriptAPI/ScriptCore.hpp"
#include <Windows.h>
#include <conio.h>

std::shared_ptr<Application> app;

extern "C"
{
	bool OpenDevice(unsigned short usVID, unsigned short usPID)
	{
		return app->openDevice(usVID, usPID);
	}

	void CloseDevice()
	{
		return app->closeDevice();
	}

	void ResetDevice()
	{
		app->getHID()->reset();
	}

	void WriteCmd(unsigned char *wbuf,int len)
	{
		app->write(wbuf, len);
	}

	void ReadCmd(unsigned char cmd, int len)
	{
		app->read(cmd, len);
	}
}

#ifdef _CONSOLE
int main (int argc, char **argv) 
{
	app.reset(&Application::instance());
	app->initialize();
	spdlog::info("\n[1] open\n[2] close\n[3] read\n[4] write\n[q] quit\n");
	while (true)
	{
		char c = getch();
		if (c == 'q')
		{
			break;
		}
		else if (c == 'h')
		{
			spdlog::info("\n[1] open\n[2] close\n[3] read\n[4] write\n[q] quit\n");
		}
		else if (c == '1')
		{
			Lua::call("test_open");
		}
		else if (c == '2')
		{
			Lua::call("test_close");
		}
		else if (c == '3')
		{
			Lua::call("test_read");
		}
		else if (c == '4')
		{
			Lua::call("test_write");
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
