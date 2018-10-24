#include "AsyncHid.hpp"
#include "Logger.hpp"
#include "hidapi.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <Windows.h>
#include <codecvt>
#include <boost/crc.hpp>

using namespace std::chrono;
#pragma warning(disable:4996)

AsyncHid::AsyncHid(void)
	:mRunning(false)
{
	mWorkerThread = std::make_shared<std::thread>(&AsyncHid::update, this);
}

void AsyncHid::init()
{
	if (hid_init())
		spdlog::error("hid init failure");
}

bool AsyncHid::open(ushort usVID, ushort usPID)
{
	mRunning = false;
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
		printf("unable to open device\n");
		return false;
	}
	mRunning = true;

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

	return mOpened;
}

void AsyncHid::close()
{
	try
	{
		mRunning = false;
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

bool AsyncHid::flush()
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

void AsyncHid::write(const Buffer&&buf)
{
	std::lock_guard<std::mutex> guard(mMutex);
	mWriteBufs.push(buf);
}

void AsyncHid::write(const Buffer&buf)
{
	std::lock_guard<std::mutex> guard(mMutex);
	mWriteBufs.push(buf);
}

void AsyncHid::read(uchar* buf, int len)
{
	if (!mOpened)
	{
		spdlog::error("read error. hid not opened");
		return;
	}
	std::lock_guard<std::mutex> guard(mMutex);
	memcpy_s(buf, len, mReadBuf.data(), mReadBuf.size());
}

void AsyncHid::read(ReadBuffer& buf)
{
	if (!mOpened)
	{
		buf = { 'h','i','d',' ','n','o','t',' ','o','p','e','n','e','d'};
		return;
	}
	std::lock_guard<std::mutex> guard(mMutex);
	buf = mReadBuf;
}

void AsyncHid::quit()
{
	mRunning = false;
}

bool AsyncHid::fetch()
{
	if (!mOpened)
		return false;

	std::lock_guard<std::mutex> guard(mMutex);

	notifyForRead();

	auto buf = mReadBuf.data();
	int res = hid_read_timeout(mHandle, buf, 65, 2000);
	if (res == 0)
		printf("waiting...\n");
	else if (res < 0)
		printf("Unable to read()\n");

	// crc16
	int len = mReadBuf[4];
	boost::crc_optimal<16, 0x1021, 0, 0, true, true> crc_ccitt_kermit;
	crc_ccitt_kermit = std::for_each(buf, buf + len + 5, crc_ccitt_kermit);
	ushort sum = crc_ccitt_kermit.checksum();
	auto high = buf[len + 5 ];
	auto low = buf[len + 5 + 1];
	ushort received_sum = (high<< 8) + low;
	if (received_sum != sum)
	{
		printf("read hid checksum failure.%d != %d\n", received_sum, sum);
		printf("%d %d %d\n", low, high, len);
	}
	return res > 0;
}

bool AsyncHid::notifyForRead()
{
	if (!mOpened)
		return false;

	//spdlog::info("notity for read");

	size_t len = 65;
	static std::array<uchar, 10> buf{0, 0x0,0x0,0x02,0x00,0x02,0x00,0xB5,0x41,0x16};
	int res = hid_write(mHandle, buf.data(), len);
	if (res < 0)
		spdlog::error("Unable to write() (2)\n");

	//auto d = buf.data();
	//spdlog::info("write buffer :{} {} {} {} {} {} {} {} {}", d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8]);

	return res > 0;
}

void AsyncHid::update()
{
	// µÈ´ý»½ÐÑ
	while (true)
	{
		while (!mRunning)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		auto start = system_clock::now();
		fetch();
		flush();
		auto end = system_clock::now();
		auto duration = duration_cast<milliseconds>(end - start);
		auto value = static_cast<long long>(std::max(0.0f, (1000.0f / mLimtedHZ) - duration.count()));
		std::this_thread::sleep_for(std::chrono::milliseconds(value));
		//spdlog::info("rw loop cost:{0:d}ms",value);
	}
}

