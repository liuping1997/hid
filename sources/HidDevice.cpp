#include "HidSdk.hpp"
#include "HidDevice.hpp"
#include "Logger.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <Windows.h>
#include <codecvt>
#include "hidapi.h"

using namespace std::chrono;
#pragma warning(disable:4996)

CHidDevice::CHidDevice(void)
{
	if (hid_init())
		spdlog::error("hid init failure");
}

CHidDevice::~CHidDevice(void)
{
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

	struct hid_device_info *devs, *cur_dev;
	devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;
	while (cur_dev) {
		printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
		printf("\n");
		printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		printf("  Product:      %ls\n", cur_dev->product_string);
		printf("  Release:      %hx\n", cur_dev->release_number);
		printf("  Interface:    %d\n", cur_dev->interface_number);
		printf("\n");
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);


	mHandle = hid_open(usVID, usPID, NULL) ;
	mOpened = mHandle > 0;
	if (!mHandle) {
		spdlog::error("unable to open device");
	}
	// Read the Manufacturer String
	int res = 0;
#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	wstr[0] = 0x0000;
	res = hid_get_manufacturer_string(mHandle, wstr, MAX_STR);
	if (res < 0)
		spdlog::info("Unable to read manufacturer string");
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	spdlog::info("Manufacturer String: {}", conv.to_bytes(std::wstring(wstr)));

	// Set the hid_read() function to be non-blocking.
	hid_set_nonblocking(mHandle, 1);

	unsigned char buf[256] = {0};
	// Send a Feature Report to the device
	buf[0] = 0x2;
	buf[1] = 0xa0;
	buf[2] = 0x0a;
	buf[3] = 0x00;
	buf[4] = 0x00;
	res = hid_send_feature_report(mHandle, buf, 65);
	if (res < 0) {
		spdlog::error("Unable to send a feature report.");
	}

	memset(buf, 0, sizeof(buf));

	// Read a Feature Report from the device
	buf[0] = 0x2;
	res = hid_get_feature_report(mHandle, buf, sizeof(buf));
	if (res < 0) {
		spdlog::error("Unable to get a feature report.");
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		spdlog::info("{}", conv.to_bytes(std::wstring(hid_error(mHandle))));
	}
	else {
		// Print out the returned buffer.
		spdlog::info("Feature Report\n   ");
		for (int i = 0; i < res; i++)
			spdlog::info("{}hhx ", buf[i]);
	}

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
		hid_close(mHandle);
		//hid_exit();
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

		auto buf = data.data();
		size_t len = data.back();
		int res = hid_write(mHandle, buf, len);

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

void CHidDevice::read(ReadBuffer& buf)
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

	notifyForRead();

	mReadBuf = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	auto buf = mReadBuf.data();
	spdlog::info("start read");
	int res = hid_read_timeout(mHandle, buf, 65, 2000);
	if (res == 0)
		spdlog::info("waiting...");
	else if (res < 0)
		spdlog::error("Unable to read()");

	auto &d = buf;
	spdlog::info("read buffer :{} {} {} {} {} {} {} {} {}", d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8]);
	return res > 0;
}

bool CHidDevice::notifyForRead()
{
	if (!mOpened)
		return false;

	spdlog::info("notity for read");

	size_t len = 65;
	static std::array<uchar, 9> buf{0x0,0x0,0x02,0x00,0x02,0x00,0xB5,0x41,0x16};
	int res = hid_write(mHandle, buf.data(), len);
	if (res < 0)
		spdlog::error("Unable to write() (2)\n");
	else
		spdlog::info("hid_write success {}",res);

	auto d = buf.data();
	spdlog::info("write buffer :{} {} {} {} {} {} {} {} {}", d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8]);

	return res > 0;
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

