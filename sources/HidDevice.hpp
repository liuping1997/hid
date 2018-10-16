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
public:
	using Buffer = std::array<unsigned char, 64>;

private:
	bool mRunning = true;
	std::mutex mMutex;
	CHidCmd *mDeviceIo = nullptr;
	bool mOpened = false;
	/// ���ݶ�дƵ��
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
	float ByteToFloat(unsigned char *pArr);
	int ByteToInt(unsigned char* pArr,int size=4);
	bool SetState(void);
	void update();
	bool fetch();
	bool flush();
};
