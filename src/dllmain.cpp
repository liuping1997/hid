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
		return gHidDevice->OpenDevice(usVID, usPID);
	}

	void CloseDevice()
	{
		if (gHidDevice == nullptr)
			return ;
		return gHidDevice->CloseDevice();
	}

	bool ResetDevice()
	{
		return true;
	}

	bool UpdateState()
	{
		if (gHidDevice == nullptr)
			return false;
		return gHidDevice->UpdateState();
	}

	bool WriteCmd(char *wbuf)
	{
		if (gHidDevice == nullptr)
			return false;
		return gHidDevice->WriteCmd(wbuf);
	}

	bool ReadCmd(char *rbuf)
	{
		if (gHidDevice == nullptr)
			return false;
		return gHidDevice->ReadCmd(rbuf);
	}

	void SendControlValue(char relay1_status, char relay2_status, short  DAC1_value, short DAC2_value, short servo_value)
	{
		if (gHidDevice == nullptr)
			return ;
		gHidDevice->SendControlValue(relay1_status, relay2_status, DAC1_value, DAC2_value, servo_value);
	}

	int GetSensorValue(int index)
	{
		if (gHidDevice == nullptr)
			return -1;
		switch (index)
		{
		case 1:
			return gHidDevice->sensorValue1;
		case 2:
			return gHidDevice->sensorValue2;
		case 3:
			return gHidDevice->sensorValue3;
		case 4:
			return gHidDevice->sensorValue4;
		case 5:
			return gHidDevice->sensorValue5;
		case 6:
			return gHidDevice->sensorValue6;
		default:
			return -1;
		}
		return -1;
	}

	int GetEncoderValue(int index)
	{
		if (gHidDevice == nullptr)
			return -1;
		switch (index)
		{
		case 1:
			return gHidDevice->encoderValue1;
		case 2:
			return gHidDevice->encoderValue2;
		case 3:
			return gHidDevice->encoderValue3;
		case 4:
			return gHidDevice->encoderValue4;
		case 5:
			return gHidDevice->encoderValue5;
		case 6:
			return gHidDevice->encoderValue6;
		default:
			return -1;
		}
		return -1;
	}


	int GetIO()
	{
		if (gHidDevice == nullptr)
			return -1;
		return  gHidDevice->switchValue_IO;
	}

	int GetOPTO()
	{
		if (gHidDevice == nullptr)
			return -1;
		return  gHidDevice->switchValue_OPTO;
	}
	
	bool GetIOById(int id)
	{
		int value = GetIO();
		switch (id)
		{
		case 1:
			return (value & 0x01) > 0;
		case 2:
			return (value & 0x02) > 0;
		case 3:
			return (value & 0x04) > 0;
		case 4:
			return (value & 0x08) > 0;
		case 5:
			return (value & 0x10) > 0;
		case 6:
			return (value & 0x20) > 0;
		case 7:
			return (value & 0x40) > 0;
		case 8:
			return (value & 0x80) > 0;
		default:
			break;
		}
		return false;
	}

	bool GetOPTOById(int id)
	{
		int value = GetOPTO();
		switch (id)
		{
		case 1:
			return (value & 0x01) > 0;
		case 2:
			return (value & 0x02) > 0;
		case 3:
			return (value & 0x04) > 0;
		case 4:
			return (value & 0x08) > 0;
		case 5:
			return (value & 0x10) > 0;
		case 6:
			return (value & 0x20) > 0;
		case 7:
			return (value & 0x40) > 0;
		case 8:
			return (value & 0x80) > 0;
		default:
			break;
		}
		return false;
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

