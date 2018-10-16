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
	HIDSDK_API void ResetDevice();
	HIDSDK_API void WriteCmd(unsigned char *cmd, int len);
	HIDSDK_API void ReadCmd(unsigned char cmd, int len);
}