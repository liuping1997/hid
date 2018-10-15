// HidSdk.cpp : Defines the exported functions for the DLL application.
//

#include <Windows.h>
#include <iostream>
#include "HidSdk.hpp"

using std::cout;
using std::endl;

#pragma warning(disable:4267)

namespace ComData {

	static unsigned char auchCRCHi[] = {
		0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
		0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
		0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
		0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
		0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
		0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,
		0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,
		0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
		0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
		0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
		0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
		0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
		0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
		0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
		0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
		0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
		0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
		0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
		0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
		0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
		0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
		0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,
		0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,
		0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
		0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
		0x80,0x41,0x00,0xC1,0x81,0x40
	};

	static unsigned char auchCRCLo[] = {
		0x00,0xC0,0xC1,0x01,0xC3,0x03,0x02,0xC2,0xC6,0x06,
		0x07,0xC7,0x05,0xC5,0xC4,0x04,0xCC,0x0C,0x0D,0xCD,
		0x0F,0xCF,0xCE,0x0E,0x0A,0xCA,0xCB,0x0B,0xC9,0x09,
		0x08,0xC8,0xD8,0x18,0x19,0xD9,0x1B,0xDB,0xDA,0x1A,
		0x1E,0xDE,0xDF,0x1F,0xDD,0x1D,0x1C,0xDC,0x14,0xD4,
		0xD5,0x15,0xD7,0x17,0x16,0xD6,0xD2,0x12,0x13,0xD3,
		0x11,0xD1,0xD0,0x10,0xF0,0x30,0x31,0xF1,0x33,0xF3,
		0xF2,0x32,0x36,0xF6,0xF7,0x37,0xF5,0x35,0x34,0xF4,
		0x3C,0xFC,0xFD,0x3D,0xFF,0x3F,0x3E,0xFE,0xFA,0x3A,
		0x3B,0xFB,0x39,0xF9,0xF8,0x38,0x28,0xE8,0xE9,0x29,
		0xEB,0x2B,0x2A,0xEA,0xEE,0x2E,0x2F,0xEF,0x2D,0xED,
		0xEC,0x2C,0xE4,0x24,0x25,0xE5,0x27,0xE7,0xE6,0x26,
		0x22,0xE2,0xE3,0x23,0xE1,0x21,0x20,0xE0,0xA0,0x60,
		0x61,0xA1,0x63,0xA3,0xA2,0x62,0x66,0xA6,0xA7,0x67,
		0xA5,0x65,0x64,0xA4,0x6C,0xAC,0xAD,0x6D,0xAF,0x6F,
		0x6E,0xAE,0xAA,0x6A,0x6B,0xAB,0x69,0xA9,0xA8,0x68,
		0x78,0xB8,0xB9,0x79,0xBB,0x7B,0x7A,0xBA,0xBE,0x7E,
		0x7F,0xBF,0x7D,0xBD,0xBC,0x7C,0xB4,0x74,0x75,0xB5,
		0x77,0xB7,0xB6,0x76,0x72,0xB2,0xB3,0x73,0xB1,0x71,
		0x70,0xB0,0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,
		0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,0x9C,0x5C,
		0x5D,0x9D,0x5F,0x9F,0x9E,0x5E,0x5A,0x9A,0x9B,0x5B,
		0x99,0x59,0x58,0x98,0x88,0x48,0x49,0x89,0x4B,0x8B,
		0x8A,0x4A,0x4E,0x8E,0x8F,0x4F,0x8D,0x4D,0x4C,0x8C,
		0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,
		0x43,0x83,0x41,0x81,0x80,0x40
	};

	//////////////////////////////////////////////////////////////////////////

};

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

BOOL CHidIO::OpenDevice(BOOL bUseTwoHandle, USHORT usVID,USHORT usPID)
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
							//FILE_ATTRIBUTE_NORMAL,
							FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
							NULL);
					m_hWriteHandle = CreateFile(MyDevPathName,
							GENERIC_WRITE,
							FILE_SHARE_READ|FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
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

BOOL CHidIO::ReadFile(char *pcBuffer, size_t szMaxLen, DWORD *pdwLength, DWORD dwMilliseconds)
{
	HANDLE events[2] = {m_hAbordEvent,m_hReadEvent};

	OVERLAPPED overlapped;
	memset(&overlapped, 0 , sizeof(overlapped));
	overlapped.hEvent = m_hReadEvent;

	if(pdwLength != NULL)
		*pdwLength = 0;
	//if(!::ReadFile(m_hReadHandle, pcBuffer, szMaxLen, NULL, &overlapped))
	//	return FALSE;
	if(!::ReadFile(m_hReadHandle, pcBuffer, szMaxLen, NULL, &overlapped))
	{

		if(ERROR_IO_PENDING == GetLastError())
		{
			DWORD dwIndex = WaitForMultipleObjects(2,events, FALSE, dwMilliseconds);

			if(dwIndex == WAIT_OBJECT_0  
				|| dwIndex == WAIT_OBJECT_0 + 1)
			{
				ResetEvent(events[dwIndex - WAIT_OBJECT_0]);

				if(dwIndex == WAIT_OBJECT_0 )
					return FALSE;   //Abort event
				else
				{
					DWORD dwLength = 0;
					//Read OK
					GetOverlappedResult(m_hReadHandle, &overlapped, &dwLength, TRUE);
					if(pdwLength != NULL)
						*pdwLength = dwLength;
					return TRUE;
				}
			}
			else
				return FALSE;
		}
		else
		{
			return FALSE;
		}
	}
	return FALSE;
}

BOOL CHidIO::WriteFile(char *pcBuffer, size_t szLen, DWORD *pdwLength, DWORD dwMilliseconds)
{
	HANDLE events[2] = {m_hAbordEvent, m_hWriteEvent};

	OVERLAPPED overlapped;
	DWORD err;

	memset(&overlapped, 0 ,sizeof(overlapped));
	overlapped.hEvent = m_hWriteEvent;

	if(pdwLength != NULL)
		*pdwLength =0;

	if(!::WriteFile(m_hWriteHandle,pcBuffer,szLen,NULL,&overlapped))
	{
		err=GetLastError();
		cout<<"Last Error is" <<err<<endl;
		return FALSE;
	}

	DWORD dwIndex = WaitForMultipleObjects(2,events,FALSE, dwMilliseconds);
	if(dwIndex == WAIT_OBJECT_0
		|| dwIndex == WAIT_OBJECT_0 +1)
	{
		ResetEvent(events[dwIndex - WAIT_OBJECT_0]);

		if(dwIndex == WAIT_OBJECT_0)
			return FALSE;// Abort event 
		else
		{
			DWORD dwLength = 0;
			//Write OK
			GetOverlappedResult(m_hWriteHandle, &overlapped, &dwLength,TRUE);
			if(pdwLength !=NULL)
				*pdwLength = dwLength;
			return TRUE;
		}
	}
	else
		return FALSE;
}


CHidCmd::CHidCmd():m_ucCmdIndex(18) //Do not use 0 to avoid firmware alrady has index 0 occasionally.
		,m_hidIO()
{
	memset(m_acBuffer,0x00,sizeof(m_acBuffer));
}

CHidCmd::~CHidCmd()
{
	
}

void  CHidCmd::CloseDevice()
{
	m_hidIO.CloseDevice();
}

BOOL  CHidCmd:: OpenDevice(USHORT usVID, USHORT usPID)
{
	return m_hidIO.OpenDevice(TRUE,usVID,usPID);
}

BOOL CHidCmd:: ReadFile(char *pcBuffer,size_t szMaxLen,DWORD *pdwLength,DWORD dwMilliseconds)
{
	DWORD dwStart = GetTickCount();
	USHORT crc=0,crc_pack=0;
	while(1)
	{
		DWORD dwLength;
		if(!m_hidIO.ReadFile(m_acBuffer,sizeof(m_acBuffer),&dwLength,dwMilliseconds))
			return FALSE;
		//Check if correct package index was read
		//m_acBuffer[0];  //For HID internal usage ,ignored
		UCHAR ucCmdIndex = ((UCHAR)m_acBuffer[2]&(UCHAR)0x7F);
		m_bCmdError = (((UCHAR)m_acBuffer[2] & (UCHAR)0x80) == (UCHAR)0x80 ? TRUE : FALSE);
		DWORD dwCmdLength = (DWORD)(UCHAR)m_acBuffer[5];
		//=====test
		if(dwLength >= 3 && ucCmdIndex == m_ucCmdIndex)
		{
			if(dwCmdLength > dwLength - 8 )
				dwCmdLength = dwLength -8;
			if(dwCmdLength > szMaxLen)
				dwCmdLength = szMaxLen;
			//CRC check
			crc = CRC16(&m_acBuffer[1],dwCmdLength+5,0xffff);
			crc_pack =(USHORT)(((UCHAR)m_acBuffer[dwCmdLength+6])<<8) | ((UCHAR)m_acBuffer[dwCmdLength+7]);
			if (crc==crc_pack)
			{
				memcpy(pcBuffer, m_acBuffer + 6, dwCmdLength);

				if(pdwLength != NULL)
					*pdwLength = dwCmdLength;
				return TRUE;
			}
			else
			{
				return FALSE;

			}
		}
	}
}

BOOL CHidCmd:: WriteFile(char *pcBuffer ,DWORD dwLen ,DWORD *pdwLength ,DWORD dwMilliseconds)
{
	USHORT crc=0;
	// Set new package index value
	++m_ucCmdIndex;
	m_ucCmdIndex = (m_ucCmdIndex & (UCHAR)0x7F);

	DWORD dwCmdLength = dwLen;
	if(dwCmdLength > sizeof(m_acBuffer) - 8)
		dwCmdLength = sizeof(m_acBuffer)- 8;
	m_acBuffer[0] = 0x00; //Always 0x00
	//package Index
	m_acBuffer[1] = 0x00;
	m_acBuffer[2] = (CHAR)m_ucCmdIndex;
	//board type
	m_acBuffer[3] = 0x02;
	//valid length
	m_acBuffer[4] = 0x00;
	m_acBuffer[5] = (UCHAR)dwCmdLength;
	//m_acBuffer[1] = (CHAR)0x12;
	//m_acBuffer[2] = (UCHAR)0x05;
	memcpy(m_acBuffer + 6, pcBuffer ,dwCmdLength);
	//新增加了CRC16校验，带2个字节 从包号开始到数据结束，随后2字节是CRC
	crc = CRC16(&m_acBuffer[1],dwCmdLength+5,0xffff);
	m_acBuffer[dwCmdLength+6]=(unsigned char)((crc&0xFF00)>>8);  
	m_acBuffer[dwCmdLength+7]=(unsigned char)(crc&0x00FF); 

	BOOL bRet = m_hidIO.WriteFile(m_acBuffer,sizeof(m_acBuffer),pdwLength,dwMilliseconds);
	return bRet;
}

BOOL CHidCmd:: IsCmdError()
{
	return m_bCmdError;
}


USHORT CHidCmd::CRC16(CHAR *puchMsgg,DWORD usDataLen,USHORT crcInput)
{
	UCHAR uchCRCHi;  
	UCHAR uchCRCLo;  
	UCHAR uIndex ; 

	uchCRCHi = (UCHAR)(crcInput>>8);
	uchCRCLo = (UCHAR)crcInput;

	while (usDataLen--) 
	{ 
		uIndex = uchCRCHi ^ *puchMsgg++ ; 
		uchCRCHi = uchCRCLo ^ ComData::auchCRCHi[uIndex] ; 
		uchCRCLo = ComData::auchCRCLo[uIndex] ; 
	} 
	return (uchCRCHi << 8 | uchCRCLo) ; 
}