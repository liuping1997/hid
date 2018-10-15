#include "Exports.hpp"
#include "HidDevice.hpp"
#include "Logger.hpp"
#include "Application.hpp"
#include <Windows.h>

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

	bool ResetDevice()
	{
		return true;
	}

	bool UpdateState()
	{
		return false;
	}

	bool WriteCmd(char *wbuf)
	{
		return false;
	}

	bool ReadCmd(char *rbuf)
	{
		return false;
	}

	int GetSensorValue(int index)
	{
		return 0;
	}

	int GetEncoderValue(int index)
	{
		return 0;
	}
}


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

