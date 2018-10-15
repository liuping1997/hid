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
	HIDSDK_API int GetSensorValue(int index);
	HIDSDK_API int GetEncoderValue(int index);
}