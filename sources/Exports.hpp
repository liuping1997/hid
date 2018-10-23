#pragma once 

#ifdef HIDSDK_EXPORTS
#define HIDSDK_API __declspec(dllexport)
#else
#define HIDSDK_API __declspec(dllimport)
#endif

extern "C"
{
	HIDSDK_API bool hidapi_open(unsigned short usVID, unsigned short usPID);
	HIDSDK_API void hidapi_close();
	HIDSDK_API void hidapi_write(const unsigned char *buf, int len);
	HIDSDK_API void hidapi_write_crc16(const unsigned char *buf, int len);
	HIDSDK_API void hidapi_read_all(unsigned char* buf, int len);
}