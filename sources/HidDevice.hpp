#pragma once 

#ifdef HIDSDK_EXPORTS
#define HIDSDK_API __declspec(dllexport)
#else
#define HIDSDK_API __declspec(dllimport)
#endif
#include <mutex>
#include <queue>
#include <array>

class CHidCmd;

class CHidDevice
{
public:
	using uchar = unsigned char;
	using Buffer = std::array<uchar, 100>;
	using ushort = unsigned short;
	using uint = unsigned int;

private:
	bool mRunning = true;
	std::mutex mMutex;
	CHidCmd *mDeviceIo = nullptr;
	bool mOpened = false;
	/// Êý¾Ý¶ÁÐ´ÆµÂÊ
	int mLimtedHZ = 20;
	Buffer mReadBuf;
	std::queue<Buffer> mWriteBufs;
	std::shared_ptr<std::thread> mWorkerThread = nullptr;

public:
	CHidDevice(void);
	~CHidDevice(void);

	bool open(unsigned short usVID, unsigned short usPID);
	void close();
	bool reset() { return true; }
	void write(const Buffer& buf);
	void write(const Buffer&& buf);
	void read(Buffer& buf);
	void quit();

private:
	bool notifyForRead();
	void update();
	bool fetch();
	bool flush();
};
