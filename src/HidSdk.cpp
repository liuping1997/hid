// HidSdk.cpp : Defines the exported functions for the DLL application.
//

#include <Windows.h>
#include <iostream>
#include "../include/HidSdk.h"
#include "../include/ComData.h"

using std::cout;
using std::endl;

// This is the constructor of a class that has been exported.
// see HidSdk.h for the class definition
CHidSdk::CHidSdk()
{
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