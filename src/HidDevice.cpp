#include "Windows.h"
#include "../include/HidSdk.h"
#include "../include/HidDevice.h"
#include <iostream>
#include <mutex>

#define USHORT unsigned short 
std::mutex gMutex;

CHidDevice::CHidDevice(void)
{
	mDeviceIo = new CHidCmd();
}

CHidDevice::~CHidDevice(void)
{
	delete mDeviceIo;
}

bool CHidDevice::open(USHORT usVID, USHORT usPID)
{
	return mDeviceIo->OpenDevice(usVID, usPID);
}

void CHidDevice::close()
{
	mDeviceIo->CloseDevice();
}

bool CHidDevice::write(char * wbuf)
{
	DWORD Length;
	return mDeviceIo->WriteFile(wbuf, 32, &Length, 2000);
}

bool CHidDevice::read(char * rbuf)
{
	std::lock_guard<std::mutex> guard(gMutex);
	DWORD Length;
	return mDeviceIo->ReadFile(rbuf, 32, &Length, 2000);
}

bool CHidDevice::read()
{
	std::lock_guard<std::mutex> guard(gMutex);

	char buffer[64]={0};
	DWORD Length;
	short tempShort;
	unsigned char c1[2];
	unsigned char b2f[4];
	SetState();
	if(!(mDeviceIo->ReadFile(buffer, sizeof(buffer), &Length, 2000)))
	{
		return false;
	}

	// Gyro 1
	b2f[1] = buffer[2];
	b2f[0] = buffer[3];
	//AHRS1_PitchValue = ByteToInt(b2f)/100.0f;

	// roll pitch .yaw
	b2f[1] = buffer[4];
	b2f[0] = buffer[5];
	//AHRS1_RollValue = ByteToInt(b2f)/100.0f;

	b2f[1] = buffer[6];
	b2f[0] = buffer[7];
	//AHRS1_YawValue = ByteToInt(b2f)/100.0f;
	
	// Gyro 2
	b2f[1] = buffer[8];
	b2f[0] = buffer[9];
	//AHRS2_PitchValue = ByteToInt(b2f)/100.0f;

	// roll pitch .yaw
	b2f[1] = buffer[10];
	b2f[0] = buffer[11];
	//AHRS2_RollValue = ByteToInt(b2f)/100.0f;

	b2f[1] = buffer[12];
	b2f[0] = buffer[13];
	//AHRS2_YawValue = ByteToInt(b2f)/100.0f;

	// Gyro 3
	b2f[1] = buffer[14];
	b2f[0] = buffer[15];
	//AHRS3_PitchValue = ByteToInt(b2f)/100.0f;

	// roll pitch .yaw
	b2f[1] = buffer[16];
	b2f[0] = buffer[17];
	//AHRS3_RollValue = ByteToInt(b2f)/100.0f;

	b2f[1] = buffer[18];
	b2f[0] = buffer[19];
	//AHRS3_YawValue = ByteToInt(b2f)/100.0f;

	//adc
	c1[1] = buffer[20];
	c1[0] = buffer[21];
	memcpy(&tempShort, &c1[0], 2);

	c1[1] = buffer[22];
	c1[0] = buffer[23];
	memcpy(&tempShort, &c1[0], 2);

	c1[1] = buffer[24];
	c1[0] = buffer[25];
	memcpy(&tempShort, &c1[0], 2);

	c1[1] = buffer[26];
	c1[0] = buffer[27];
	memcpy(&tempShort, &c1[0], 2);

	c1[1] = buffer[28];
	c1[0] = buffer[29];
	memcpy(&tempShort, &c1[0], 2);

	c1[1] = buffer[30];
	c1[0] = buffer[31];
	memcpy(&tempShort, &c1[0], 2);

	c1[1] = buffer[32];
	c1[0] = buffer[33];
	memcpy(&tempShort, &c1[0], 2);

	//switchValue_OPTO = buffer[34];
	//switchValue_IO =  buffer[35];
	
	c1[1] = buffer[36];
	c1[0] = buffer[37];
	memcpy(&tempShort, &c1[0], 2);

	c1[1] = buffer[38];
	c1[0] = buffer[39];
	memcpy(&tempShort, &c1[0], 2);

	c1[1] = buffer[40];
	c1[0] = buffer[41];
	memcpy(&tempShort, &c1[0], 2);

	c1[1] = buffer[42];
	c1[0] = buffer[43];
	memcpy(&tempShort, &c1[0], 2);

	c1[1] = buffer[44];
	c1[0] = buffer[45];
	memcpy(&tempShort, &c1[0], 2);

	return true;
}

bool CHidDevice::SetState()
{
	unsigned char xBuf[2];

	DWORD Length;
	xBuf[0] = 0x00;
	xBuf[1] = 0xB5;
	if (!mDeviceIo->WriteFile((char *)&xBuf, sizeof(xBuf), &Length, 2000))
	{
		mDeviceIo->OpenDevice(0x051A, 0x511B);
		return mDeviceIo->WriteFile((char *)&xBuf, sizeof(xBuf), &Length, 2000);
	}
	else
	{
		return true;
	}
}


float CHidDevice::ByteToFloat(unsigned char *pArr)
{
	byte2float data;
	data.float_byte.high_byte  = *(pArr++);
	data.float_byte.mhigh_byte = *(pArr++);
	data.float_byte.mlow_byte  = *(pArr++);
	data.float_byte.low_byte   = *pArr;

	return data.value;
}

int CHidDevice::ByteToInt(unsigned char* pArr,int size)
{
	byte2short data;
	data.short_byte.high_byte  = *(pArr++);
	data.short_byte.low_byte   = *pArr;
	return data.value;
}