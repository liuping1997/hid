// HidSdk.cpp : Defines the exported functions for the DLL application.
//

#include "HidSdk.hpp"
#include "Logger.hpp"
#include  <boost/crc.hpp>
#include <Windows.h>
#include <iostream>
#include <codecvt>
#include "hidapi.h"

using std::cout;
using std::endl;

#pragma warning(disable:4267)
#pragma warning(disable:4996)

std::string getLastErrorInfo()
{
	DWORD errorcode = GetLastError();
	HLOCAL LocalAddress = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errorcode, 0, (LPWSTR)&LocalAddress, 0, NULL);

	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	std::wstring wstr(static_cast<LPWSTR>(LocalAddress));
	return (conv.to_bytes( wstr));
}

CHidIO::CHidIO():m_hReadHandle(INVALID_HANDLE_VALUE)
		,m_hWriteHandle(INVALID_HANDLE_VALUE)
		,m_hAbordEvent(CreateEvent(NULL,TRUE,FALSE,NULL))
		,m_hReadEvent(CreateEvent(NULL,TRUE,FALSE,NULL))
		,m_hWriteEvent(CreateEvent(NULL,0,0,NULL))
		,m_bUseTwoHandle(TRUE)
{
}

CHidIO:: ~CHidIO()
{
	CloseDevice();
		
	CloseHandle(m_hWriteEvent);
	CloseHandle(m_hReadEvent);
	CloseHandle(m_hAbordEvent);
}

void CHidIO::CloseDevice()
{
	if(m_bUseTwoHandle)
	{
		if(m_hReadHandle != INVALID_HANDLE_VALUE)
			CancelIo(m_hReadHandle);
		if(m_hWriteHandle != INVALID_HANDLE_VALUE)
			CancelIo(m_hWriteHandle);
		if(m_hReadHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hReadHandle);
			m_hReadHandle = INVALID_HANDLE_VALUE;
		}
		if(m_hWriteHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hWriteHandle);
			m_hWriteHandle = INVALID_HANDLE_VALUE;
		}
	}
	else
	{
		if(m_hReadHandle != INVALID_HANDLE_VALUE)
			CancelIo(m_hReadHandle);
		if(m_hReadHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_hReadHandle);
			m_hReadHandle =  INVALID_HANDLE_VALUE;
			m_hWriteHandle =  INVALID_HANDLE_VALUE;
		}
	}
}

BOOL CHidIO::OpenDevice(BOOL bUseTwoHandle, ushort usVID,ushort usPID)
{
	m_bUseTwoHandle = bUseTwoHandle;

	TCHAR MyDevPathName[MAX_PATH];

	//定义一个GUID的结构体HidGuid来保存HID设备的接口类GUID
	GUID HidGuid;
	//定义一个DEVINFO的句柄hDevInfoSet来保存获取到的设备信息集
	HDEVINFO hDevInfoSet;
	//定义MemberIndex,表示当前搜索到第几个设备,0表示第一个设备
	DWORD MemberIndex;
	//DevInterfaceData,用来保存设备的驱动接口信息
	SP_DEVICE_INTERFACE_DATA DevInterfaceData;
	//定义一个BOOL变量,保存设备的驱动接口信息
	BOOL Result;
	//定义一个RequiredSize的变量,用来接收需要保存详细信息的缓冲长度.
	DWORD RequiredSize;
	//定义一个指向设备详细信息的结构体指针
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDevDetailData;
	//定义一个用来保存打开设备的句柄
	HANDLE hDevHandle;
	//定义一个HIDD_ATTRIBUTES的结构体变量,保存设备的属性.
	HIDD_ATTRIBUTES DevAttributes;
	//初始化未找到
	BOOL MyDevFound = FALSE;

	//初始化读 写句柄为无效句柄
	m_hReadHandle = INVALID_HANDLE_VALUE;
	m_hWriteHandle = INVALID_HANDLE_VALUE;

	//对DevInterfaceData结构体的cbSize初始化为结构体大小
	DevInterfaceData.cbSize = sizeof(DevInterfaceData);
	//对DevAttribute结构体的size初始化为结构体大小
	DevAttributes.Size = sizeof(DevAttributes);
	//调用HidGuid来获取设备HID设备的GUID,并保存在HidGuid中.
	HidD_GetHidGuid(&HidGuid);
	//根据HidGuid来获取设备信息集合,其中Flags参数设置为
	//DIGCF_DEVICEINTERFACE|DIGCF_PRESENT,前者表示使用的GUID为
	//接口类GUID,后者表示只列举正在使用的设备,因为我们这里只
	//查找已经连接上的设备.返回的句柄保存在hDevinfo中,注意设备
	//信息集合在使用完毕后,要使用函数SetupDiDestroyDeviceInfoList
	//销毁,不然会造成内存泄漏.
	hDevInfoSet = SetupDiGetClassDevs(&HidGuid,
		NULL,
		NULL,
		DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);
	//AddToInfOut("开始查找设备");
	//然后对设备集合中每个设备进行列举,检查是否是我们要找的设备
	//当找到我们指定的设备,或着设备已经查找完毕时,就退出查找.
	//首先指向第一个设备,即将MemberIndex置为0.
	MemberIndex = 0;
	while(1)
	{
		//调用SetupDiEnumDeviceInterface在设备信息集合中获取编号为
		//MemberIndex的设备信息.
		Result = SetupDiEnumDeviceInterfaces(hDevInfoSet,
			NULL,
			&HidGuid,
			MemberIndex,
			&DevInterfaceData);
		//如果获取下信息失败,则说明设备已经查找完毕,退出循环.
		if(Result==FALSE) break;
		//将MemberIndex指向下一个设备
		MemberIndex++;
		//如果获取信息成功,则继续获取该设备的详细信息,在获取信息时
		//需呀先知道保存详细信息需要多大的缓冲区,这通过第一次调用函数
		//SetupDiGetDeviceInterfaceDetail来获取.这时
		//提供缓冲区和长度都为NULL的参数,并提供一个用来保存需要多大缓冲
		//区的变量RequiredSize.
		Result = SetupDiGetDeviceInterfaceDetail(hDevInfoSet,
			&DevInterfaceData,
			NULL,
			NULL,
			&RequiredSize,
			NULL);
		//然后,分配一个大小为RequiredSize的缓冲区,
		//用来保存设备详细信息.
		pDevDetailData = 
			(PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);
		if(pDevDetailData == NULL)//如果内存不足,则直接返回.
		{
			//MessageBox("内存不足!");
			SetupDiDestroyDeviceInfoList(hDevInfoSet);
			return FALSE;
		}

		//并设置pDevDetailData的cbsize为结构体的大小(注意只是结构体大小
		//,不包括后面缓冲区).
		pDevDetailData->cbSize = 
			sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		//然后再次调用
		//SetupDiGetDeviceInterfaceDetail函数来获取设备的
			//详细信息.这次调用设置使用的缓冲区以及缓冲区大小.
		Result = SetupDiGetDeviceInterfaceDetail(
			hDevInfoSet,
			&DevInterfaceData,
			pDevDetailData,
			RequiredSize,
			NULL,
			NULL);
		//将设备路径复制出来,然后销毁刚刚申请的内存.
		//MyDevPathName = pDevDetailData->DevicePath;
		//_tcscpy((char*)MyDevPathName,(char*)pDevDetailData->DevicePath );
		strcpy_s((char*)MyDevPathName,sizeof(MyDevPathName),(char*)pDevDetailData->DevicePath );
		free(pDevDetailData);
		//如果调用失败,则查找下一个设备.
		if(Result == FALSE) continue;
		//如果调用成功,则使用不带读写访问的CreateFile函数
		//来获取设备的属性,包括VID PID 版本号等.
		//对于一些独占设备(例如usb键盘),使用读写访问访问方式是无法打开的
		//而使用不带读写访问的格式才可以打开这些设备,从而获取设备的属性.
		hDevHandle = CreateFile (MyDevPathName,
			NULL,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);	
		
		//如果打开成功,则获取设备属性
		if(hDevHandle != INVALID_HANDLE_VALUE)
		{
			//获取设备的属性并保存在DevAttributes结构体中
			Result = HidD_GetAttributes(hDevHandle,
				&DevAttributes);
			//关闭刚刚打开的设备
			CloseHandle(hDevHandle);

			//获取失败,查找下一个
			if(Result == FALSE) continue;

			//如果获取成功,则将属性中的VID/PId与我们需要的
			//进行比较,如果一致的话,则说明它就是我们要找的设备.
			if(DevAttributes.VendorID == usVID&&
				DevAttributes.ProductID == usPID){
				MyDevFound = TRUE;
				if(bUseTwoHandle)
				{
					m_hReadHandle = CreateFile(MyDevPathName,
							GENERIC_READ,
							FILE_SHARE_READ|FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							//FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
							NULL);
					m_hWriteHandle = CreateFile(MyDevPathName,
							GENERIC_WRITE,
							FILE_SHARE_READ|FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

					spdlog::info("CreatFile For Read: {}", m_hReadEvent > 0);
					spdlog::info("CreatFile For Write: {}", m_hWriteEvent > 0);
				}
				else
				{
					m_hWriteHandle =
					m_hReadHandle = CreateFile(MyDevPathName,
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ|FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
				}
				break;
			}
		}
		//如果打开失败,则查找下一个设备
		else continue;
	}
	//调用SetupDiDestroyDeviceInfoList函数销毁设备信息集合
	SetupDiDestroyDeviceInfoList(hDevInfoSet);

	//如果设备已经找到,那么应该使能各种操作按钮,并同时禁止打开设备按钮
	return MyDevFound;
}

BOOL CHidIO::ReadFile(uchar *pcBuffer, size_t szLen, DWORD *pdwLength, DWORD dwMilliseconds)
{
	HANDLE events[2] = {m_hAbordEvent,m_hReadEvent};

	OVERLAPPED overlapped;
	memset(&overlapped, 0 , sizeof(overlapped));
	overlapped.hEvent = m_hReadEvent;
	overlapped.Offset = 0;
	overlapped.OffsetHigh = 0;

	if(pdwLength != NULL)
		*pdwLength = 0;

	szLen = ((szLen / 64) + 1) * 64 + 1;

	uchar d[128] = { 0 };
	spdlog::info("read buffer 1:{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}", d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
	if(!::ReadFile(m_hReadHandle, d, szLen, NULL, NULL))
	{
		if (ERROR_IO_PENDING == GetLastError())
		{
			DWORD waitError = 0;
			int count = 0;
			do
			{
				if (++count > 5)
					break;
				waitError = WaitForSingleObject(m_hReadEvent, dwMilliseconds);
				spdlog::info("ReadFile WaitForSingleObject");
			} while (waitError == WAIT_TIMEOUT);

			//DWORD dwIndex = WaitForMultipleObjects(2, events, FALSE, dwMilliseconds);
			if(waitError== WAIT_OBJECT_0 || waitError == WAIT_OBJECT_0 + 1)
			{
				ResetEvent(overlapped.hEvent);

				if (waitError == WAIT_OBJECT_0 + 1)
				{
					spdlog::error("abort event");
					return FALSE;   //Abort event
				}
				else if (waitError == (WAIT_OBJECT_0))
				{
					//DWORD dwLength = 0;
					////Read OK
					//GetOverlappedResult(m_hReadHandle, &overlapped, &dwLength, TRUE);
					//if(pdwLength != NULL)
					//	*pdwLength = dwLength;
					spdlog::info("read buffer 2:{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}", d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
					return TRUE;
				}
			}
			else
			{
				if (waitError == WAIT_TIMEOUT)
					spdlog::error("ReadFile WaitForMultipleObjects WAIT_TIMEOUT");
				else if (waitError == WAIT_FAILED)
					spdlog::error("ReadFile WaitForMultipleObjects WAIT_FAILED");
				spdlog::error("ReadFile :{} {}", getLastErrorInfo(), szLen);
				return FALSE;
			}
		}
		else
		{
			spdlog::error("ReadFile :{}", getLastErrorInfo());
			return FALSE;
		}
	}
	spdlog::info("ReadFile success");
	return TRUE;
}

BOOL CHidIO::WriteFile(uchar *pcBuffer, size_t szLen, DWORD *pdwLength, DWORD dwMilliseconds)
{
	HANDLE events[2] = {m_hAbordEvent, m_hWriteEvent};

	OVERLAPPED overlapped;

	memset(&overlapped, 0 ,sizeof(overlapped));
	overlapped.hEvent = m_hWriteEvent;

	if(pdwLength != NULL)
		*pdwLength =0;

	szLen = ((szLen / 64) + 1) * 64 +1;

	DWORD dwWrite = 0;
	if(!::WriteFile(m_hWriteHandle,pcBuffer,szLen,&dwWrite,&overlapped))
	{
		spdlog::error("error info:{} len:{}", getLastErrorInfo(), szLen);
		return FALSE;
	}

	DWORD dwIndex = WaitForMultipleObjects(2,events,FALSE, dwMilliseconds);
	if(dwIndex == WAIT_OBJECT_0
		|| dwIndex == WAIT_OBJECT_0 +1)
	{
		ResetEvent(events[dwIndex - WAIT_OBJECT_0]);

		if (dwIndex == WAIT_OBJECT_0)
		{
			return FALSE;// Abort event 
		}
		else
		{
			DWORD dwLength = 0;
			//Write OK
			GetOverlappedResult(m_hWriteHandle, &overlapped, &dwLength,TRUE);
			if(pdwLength !=NULL)
				*pdwLength = dwLength;
			auto d = pcBuffer;
			spdlog::info("write buffer :{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}", d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
			spdlog::info("WriteFile success");
			return TRUE;
		}
	}
	else
	{
		if (dwIndex == WAIT_TIMEOUT)
			spdlog::error("ReadFile WaitForMultipleObjects WAIT_TIMEOUT");
		else if (dwIndex == WAIT_FAILED)
			spdlog::error("ReadFile WaitForMultipleObjects WAIT_FAILED");
		spdlog::error("ReadFile :{} {}", getLastErrorInfo(), szLen);
		return FALSE;
	}
}


CHidCmd::CHidCmd()
		:m_ucCmdIndex(18) //Do not use 0 to avoid firmware alrady has index 0 occasionally.
		,m_hidIO()
{
	memset(m_acBuffer, 0x00, sizeof(m_acBuffer));
}

CHidCmd::~CHidCmd()
{
	
}

void  CHidCmd::CloseDevice()
{
	m_hidIO.CloseDevice();
}

BOOL  CHidCmd:: OpenDevice(ushort usVID, ushort usPID)
{
	return m_hidIO.OpenDevice(TRUE,usVID,usPID);
}

BOOL CHidCmd:: ReadFile(uchar *pcBuffer,size_t szMaxLen,DWORD *pdwLength,DWORD dwMilliseconds)
{
	DWORD dwStart = GetTickCount();
	ushort crc=0,crc_pack=0;
	while(1)
	{
		DWORD dwLength;
		m_acBuffer[0] = 0; 
		m_acBuffer[1] = 0; 
		m_acBuffer[2] = 0x02; 
		m_acBuffer[4] = szMaxLen; 
		m_acBuffer[5] = 0; 

		memset(m_acBuffer, 0, sizeof(m_acBuffer));

		uchar d[512] = {};

		if (!m_hidIO.ReadFile(d, 64, &dwLength, dwMilliseconds))
		{
			return FALSE;
		}
		spdlog::info("read buffer {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}",d[0],d[1],d[2],d[3],d[4],d[5],d[6],d[7],d[8],d[9],d[10],d[11],d[12],d[13],d[14],d[15]);

		uchar ucCmdIndex = ((uchar)m_acBuffer[2]&(uchar)0x7F);
		m_bCmdError = (((uchar)m_acBuffer[2] & (uchar)0x80) == (uchar)0x80 ? TRUE : FALSE);
		DWORD dwCmdLength = (DWORD)(uchar)m_acBuffer[5];
		if(dwLength >= 3 && ucCmdIndex == m_ucCmdIndex)
		{
			if(dwCmdLength > dwLength - 8 )
				dwCmdLength = dwLength -8;
			if(dwCmdLength > szMaxLen)
				dwCmdLength = szMaxLen;

			//CRC check
			crc = CRC16(m_acBuffer,dwCmdLength + 5);
			crc_pack =(ushort)(((uchar)m_acBuffer[dwCmdLength+6])<<8) | ((uchar)m_acBuffer[dwCmdLength+5]);
			if (crc==crc_pack)
			{
				memcpy(pcBuffer, m_acBuffer + 6, dwCmdLength);

				if(pdwLength != NULL)
					*pdwLength = dwCmdLength;
				return TRUE;
			}
			else
			{
				spdlog::info("crc check failure:{} != {}", crc, crc_pack);
				return FALSE;
			}
		}
	}
}

BOOL CHidCmd:: WriteFile(uchar *pcBuffer ,DWORD dwLen ,DWORD *pdwLength ,DWORD dwMilliseconds)
{
	++m_ucCmdIndex;
	m_ucCmdIndex = (m_ucCmdIndex & (uchar)0x7F);

	int len = std::clamp(static_cast<int>(dwLen),1,64);

	uchar* buf = m_acBuffer;
	buf[0] = 0x00; 
	buf[1] = 0;
	buf[2] = 0x02;
	buf[3] = 0;
	buf[4] = (uchar)len;
	buf[5] = 0;
	buf[6] = 0;
	memcpy(buf + 6, pcBuffer ,len - 1);
	ushort crc = CRC16(buf,len + 5);
	buf[len+5]=(uchar)((crc&0xFF00)>>8);
	buf[len+6]=(uchar)(crc&0x00FF); 

	uchar* d = buf;
	BOOL bRet = m_hidIO.WriteFile(buf, dwLen + 7, pdwLength, dwMilliseconds);
	if (!bRet)
	{
		spdlog::error("[[ {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} ]]", d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
		spdlog::error("write failure-->{} {}", getLastErrorInfo(),len);
	}
	return bRet;
}

BOOL CHidCmd:: IsCmdError()
{
	return m_bCmdError;
}

ushort CHidCmd::CRC16(uchar* data, DWORD len)
{
	/*
	name    polynomial  initial val  reverse byte ? reverse result ? swap result ?
		CCITT         1021         ffff             no               no            no
		XModem        1021         0000             no               no            no
		Kermit        1021         0000            yes              yes           yes
		CCITT 1D0F    1021         1d0f             no               no            no
		IBM           8005         0000            yes              yes            no
	*/
	boost::crc_optimal<16, 0x1021, 0, 0, true, true> crc_ccitt_kermit;
	crc_ccitt_kermit = std::for_each(data, data + len, crc_ccitt_kermit);
	return crc_ccitt_kermit.checksum();
}