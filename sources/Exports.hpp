#pragma once 

#ifdef HIDSDK_EXPORTS
#define HIDSDK_API __declspec(dllexport)
#else
#define HIDSDK_API __declspec(dllimport)
#endif

extern "C"
{
	HIDSDK_API bool usb_hid_open(unsigned short usVID, unsigned short usPID);
	HIDSDK_API void usb_hid_close();
	HIDSDK_API void usb_hid_write(const unsigned char *buf, int len);
	HIDSDK_API void usb_hid_read_all(unsigned char* buf, int len);
	HIDSDK_API int usb_hid_read(int id, int mask);
}