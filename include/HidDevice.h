#pragma once 

#ifdef HIDSDK_EXPORTS
#define HIDSDK_API __declspec(dllexport)
#else
#define HIDSDK_API __declspec(dllimport)
#endif
#include <mutex>
#include <Windows.h>

class CHidCmd;

typedef union {
	struct
	{
		unsigned char low_byte;
		unsigned char mlow_byte;
		unsigned char mhigh_byte;
		unsigned char high_byte;
	}float_byte;
	struct
	{
		unsigned short int low_word;
		unsigned short int high_word;
	}float_word;
	float  value;
}byte2float;

typedef union {
	struct
	{
		unsigned char low_byte;
		unsigned char high_byte;
	}short_byte;
	short int  value;
}byte2short;


class CHidDevice
{
private:
	CHidCmd *mDeviceIo;

public:
	CHidDevice(void);
	~CHidDevice(void);

	bool open(unsigned short usVID, unsigned short usPID);
	void close();
	bool reset();
	bool read();
	bool write(char *wbuf);
	bool read(char *rbuf);

private:
	float ByteToFloat(unsigned char *pArr);
	int ByteToInt(unsigned char* pArr,int size=4);
	bool SetState(void);

};
