#pragma once 

#ifdef HIDSDK_EXPORTS
#define HIDSDK_API __declspec(dllexport)
#else
#define HIDSDK_API __declspec(dllimport)
#endif

extern "C"
{
	HIDSDK_API bool OpenDevice(unsigned short usVID, unsigned short usPID);
	HIDSDK_API void CloseDevice();
	HIDSDK_API bool ResetDevice();
	HIDSDK_API bool UpdateState();
	HIDSDK_API bool WriteCmd(char *wbuf);
	HIDSDK_API bool ReadCmd(char *rbuf);
	HIDSDK_API void SendControlValue(char relay1_status, char relay2_status, short  DAC1_value, short DAC2_value, short servo_value);
	HIDSDK_API int GetSensorValue(int index);
	HIDSDK_API int GetEncoderValue(int index);
	HIDSDK_API int GetIO();
	HIDSDK_API int GetOPTO();
	HIDSDK_API bool GetIOById(int id);
	HIDSDK_API bool GetOPTOById(int id);
}