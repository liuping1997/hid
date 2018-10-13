// dllmain.cpp : Defines the entry point for the DLL application.
#include "../include/Exports.h"
#include "../include/HidDevice.h"
#include "Windows.h"

CHidDevice* gHidDevice = nullptr;

extern "C"
{
	bool OpenDevice(unsigned short usVID, unsigned short usPID)
	{
		if (gHidDevice == nullptr)
			return false;
		return gHidDevice->open(usVID, usPID);
	}

	void CloseDevice()
	{
		if (gHidDevice == nullptr)
			return ;
		return gHidDevice->close();
	}

	bool ResetDevice()
	{
		return true;
	}

	bool UpdateState()
	{
		if (gHidDevice == nullptr)
			return false;
		return gHidDevice->read();
	}

	bool WriteCmd(char *wbuf)
	{
		if (gHidDevice == nullptr)
			return false;
		return gHidDevice->write(wbuf);
	}

	bool ReadCmd(char *rbuf)
	{
		if (gHidDevice == nullptr)
			return false;
		return gHidDevice->read(rbuf);
	}

	int GetSensorValue(int index)
	{
		if (gHidDevice == nullptr)
			return -1;
		return 0;
	}

	int GetEncoderValue(int index)
	{
		return 0;
	}
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		gHidDevice = new CHidDevice();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		delete gHidDevice;
		gHidDevice = nullptr;
		break;
	}
	return TRUE;
}

