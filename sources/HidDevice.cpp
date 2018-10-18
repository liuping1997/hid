#include "HidSdk.hpp"
#include "HidDevice.hpp"
#include "Logger.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <Windows.h>

using namespace std::chrono;

CHidDevice::CHidDevice(void)
{
	mDeviceIo = new CHidCmd();
}

CHidDevice::~CHidDevice(void)
{
	delete mDeviceIo;
}

bool CHidDevice::open(ushort usVID, ushort usPID)
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

	DWORD Length;
	notifyForRead();
	if(!(mDeviceIo->ReadFile(mReadBuf.data() + 6, mReadBuf.back(), &Length, 500)))
	{
		return false;
	}
	return true;
}

bool CHidDevice::notifyForRead()
{
	if (!mOpened)
		return false;

	DWORD Length;
	uchar xBuf = 0xB5;
	spdlog::info("send read request");
	if (!mDeviceIo->WriteFile(&xBuf, 2, &Length, 2000))
	{
		spdlog::error("send read request failure");
		mOpened = mDeviceIo->OpenDevice(0x051A, 0x511B);
		return mDeviceIo->WriteFile(&xBuf, sizeof(xBuf) + 1, &Length, 500);
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

