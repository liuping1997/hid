#include "HidSdk.hpp"
#include "HidDevice.hpp"
#include "Logger.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <Windows.h>

#define USHORT unsigned short 
using namespace std::chrono;

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
	if (mWorkerThread != nullptr)
	{
		if (mRunning && mWorkerThread->joinable())
		{
			mRunning = false;
			mWorkerThread->join();
		}
	}
	mRunning = true;
	mWorkerThread = std::make_shared<std::thread>(&CHidDevice::update, this);
	mOpened =  mDeviceIo->OpenDevice(usVID, usPID);
	return mOpened;
}

void CHidDevice::close()
{
	try
	{
		if (mRunning &&mWorkerThread != nullptr)
		{
			mRunning = false;
			mWorkerThread->join();
		}
		std::lock_guard<std::mutex> guard(mMutex);
		mDeviceIo->CloseDevice();
		mOpened = false;
	}
	catch (std::system_error &e)
	{
		spdlog::error(e.what());
	}
}

bool CHidDevice::flush()
{
	if (!mOpened)
		return false;

	std::lock_guard<std::mutex> guard(mMutex);
	while (mWriteBufs.size() > 0)
	{
		auto data = mWriteBufs.front();
		DWORD Length;
		mDeviceIo->WriteFile(data.data(), data.at(63), &Length, 2000);
		mWriteBufs.pop();
	}
	return true;
}

void CHidDevice::write(const Buffer&&buf)
{
	std::lock_guard<std::mutex> guard(mMutex);
	mWriteBufs.push(buf);
}

void CHidDevice::write(const Buffer&buf)
{
	std::lock_guard<std::mutex> guard(mMutex);
	mWriteBufs.push(buf);
}

void CHidDevice::read(Buffer& buf)
{
	if (!mOpened)
	{
		buf = { 'h','i','d',' ','n','o','t',' ','o','p','e','n','e','d'};
		return;
	}
	std::lock_guard<std::mutex> guard(mMutex);
	buf = mReadBuf;
}

void CHidDevice::quit()
{
	mRunning = false;
	if (mWorkerThread != nullptr && mWorkerThread->joinable())
		mWorkerThread->join();
}

bool CHidDevice::fetch()
{
	if (!mOpened)
		return false;

	std::lock_guard<std::mutex> guard(mMutex);

	auto buffer = mReadBuf.data();
	DWORD Length;
	short tempShort;
	unsigned char c1[2];
	unsigned char b2f[4];
	SetState();
	if(!(mDeviceIo->ReadFile(mReadBuf.data() + 6, mReadBuf.back(), &Length, 500)))
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
	if (!mOpened)
		return false;

	DWORD Length;
	unsigned char xBuf = 0xB5;
	spdlog::info("send read request");
	if (!mDeviceIo->WriteFile(&xBuf, 2, &Length, 2000))
	{
		spdlog::error("send read request failure");
		mOpened = mDeviceIo->OpenDevice(0x051A, 0x511B);
		return mDeviceIo->WriteFile(&xBuf, sizeof(xBuf) + 1, &Length, 2000);
	}
	else
	{
		return true;
	}
}

void CHidDevice::update()
{
	// µÈ´ý»½ÐÑ
	while (!mRunning)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	while (mRunning)
	{
		auto start = system_clock::now(); 
		fetch();
		flush();
		auto end = system_clock::now(); 
		auto duration = duration_cast<milliseconds>(end - start);
		auto value = static_cast<long long>(max(0, 1000.0f / mLimtedHZ - duration.count()));
		std::this_thread::sleep_for(std::chrono::milliseconds(value));
		//spdlog::info("rw loop cost:{0:d}ms",value);
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