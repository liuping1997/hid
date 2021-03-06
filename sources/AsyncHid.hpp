#pragma once 

#ifdef HIDSDK_EXPORTS
#define HIDSDK_API __declspec(dllexport)
#else
#define HIDSDK_API __declspec(dllimport)
#endif
#include <mutex>
#include <queue>
#include <array>
#include "hidapi.h"

class AsyncHid
{
public:
	using uchar = unsigned char;
	using Buffer = std::array<uchar, 128>;
	using ReadBuffer = std::array<uchar, 1024>;
	using ushort = unsigned short;
	using uint = unsigned int;

private:
	hid_device*  mHandle;
	bool mRunning = true;
	std::mutex mMutex;
	bool mOpened = false;
	/// ���ݶ�дƵ��
	int mLimtedHZ = 20;
	ReadBuffer mReadBuf;
	std::queue<Buffer> mWriteBufs;
	std::shared_ptr<std::thread> mWorkerThread = nullptr;

public:
	AsyncHid();

	static AsyncHid& Get()
	{
		static AsyncHid m_instance;
		return m_instance;
	}
	void init();
	bool open(ushort usVID, ushort usPID);
	void close();
	void write(const Buffer& buf);
	void write(const Buffer&& buf);
	void read(ReadBuffer& buf);
	void read(uchar* buf, int len);
	void quit();

private:
	bool notifyForRead();
	void update();
	bool fetch();
	bool flush();
};
