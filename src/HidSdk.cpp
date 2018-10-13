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

	//����һ��GUID�Ľṹ��HidGuid������HID�豸�Ľӿ���GUID
	GUID HidGuid;
	//����һ��DEVINFO�ľ��hDevInfoSet�������ȡ�����豸��Ϣ��
	HDEVINFO hDevInfoSet;
	//����MemberIndex,��ʾ��ǰ�������ڼ����豸,0��ʾ��һ���豸
	DWORD MemberIndex;
	//DevInterfaceData,���������豸�������ӿ���Ϣ
	SP_DEVICE_INTERFACE_DATA DevInterfaceData;
	//����һ��BOOL����,�����豸�������ӿ���Ϣ
	BOOL Result;
	//����һ��RequiredSize�ı���,����������Ҫ������ϸ��Ϣ�Ļ��峤��.
	DWORD RequiredSize;
	//����һ��ָ���豸��ϸ��Ϣ�Ľṹ��ָ��
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDevDetailData;
	//����һ������������豸�ľ��
	HANDLE hDevHandle;
	//����һ��HIDD_ATTRIBUTES�Ľṹ�����,�����豸������.
	HIDD_ATTRIBUTES DevAttributes;
	//��ʼ��δ�ҵ�
	BOOL MyDevFound = FALSE;

	//��ʼ���� д���Ϊ��Ч���
	m_hReadHandle = INVALID_HANDLE_VALUE;
	m_hWriteHandle = INVALID_HANDLE_VALUE;

	//��DevInterfaceData�ṹ���cbSize��ʼ��Ϊ�ṹ���С
	DevInterfaceData.cbSize = sizeof(DevInterfaceData);
	//��DevAttribute�ṹ���size��ʼ��Ϊ�ṹ���С
	DevAttributes.Size = sizeof(DevAttributes);
	//����HidGuid����ȡ�豸HID�豸��GUID,��������HidGuid��.
	HidD_GetHidGuid(&HidGuid);
	//����HidGuid����ȡ�豸��Ϣ����,����Flags��������Ϊ
	//DIGCF_DEVICEINTERFACE|DIGCF_PRESENT,ǰ�߱�ʾʹ�õ�GUIDΪ
	//�ӿ���GUID,���߱�ʾֻ�о�����ʹ�õ��豸,��Ϊ��������ֻ
	//�����Ѿ������ϵ��豸.���صľ��������hDevinfo��,ע���豸
	//��Ϣ������ʹ����Ϻ�,Ҫʹ�ú���SetupDiDestroyDeviceInfoList
	//����,��Ȼ������ڴ�й©.
	hDevInfoSet = SetupDiGetClassDevs(&HidGuid,
		NULL,
		NULL,
		DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);
	//AddToInfOut("��ʼ�����豸");
	//Ȼ����豸������ÿ���豸�����о�,����Ƿ�������Ҫ�ҵ��豸
	//���ҵ�����ָ�����豸,�����豸�Ѿ��������ʱ,���˳�����.
	//����ָ���һ���豸,����MemberIndex��Ϊ0.
	MemberIndex = 0;
	while(1)
	{
		//����SetupDiEnumDeviceInterface���豸��Ϣ�����л�ȡ���Ϊ
		//MemberIndex���豸��Ϣ.
		Result = SetupDiEnumDeviceInterfaces(hDevInfoSet,
			NULL,
			&HidGuid,
			MemberIndex,
			&DevInterfaceData);
		//�����ȡ����Ϣʧ��,��˵���豸�Ѿ��������,�˳�ѭ��.
		if(Result==FALSE) break;
		//��MemberIndexָ����һ���豸
		MemberIndex++;
		//�����ȡ��Ϣ�ɹ�,�������ȡ���豸����ϸ��Ϣ,�ڻ�ȡ��Ϣʱ
		//��ѽ��֪��������ϸ��Ϣ��Ҫ���Ļ�����,��ͨ����һ�ε��ú���
		//SetupDiGetDeviceInterfaceDetail����ȡ.��ʱ
		//�ṩ�������ͳ��ȶ�ΪNULL�Ĳ���,���ṩһ������������Ҫ��󻺳�
		//���ı���RequiredSize.
		Result = SetupDiGetDeviceInterfaceDetail(hDevInfoSet,
			&DevInterfaceData,
			NULL,
			NULL,
			&RequiredSize,
			NULL);
		//Ȼ��,����һ����СΪRequiredSize�Ļ�����,
		//���������豸��ϸ��Ϣ.
		pDevDetailData = 
			(PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);
		if(pDevDetailData == NULL)//����ڴ治��,��ֱ�ӷ���.
		{
			//MessageBox("�ڴ治��!");
			SetupDiDestroyDeviceInfoList(hDevInfoSet);
			return FALSE;
		}

		//������pDevDetailData��cbsizeΪ�ṹ��Ĵ�С(ע��ֻ�ǽṹ���С
		//,���������滺����).
		pDevDetailData->cbSize = 
			sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		//Ȼ���ٴε���
		//SetupDiGetDeviceInterfaceDetail��������ȡ�豸��
			//��ϸ��Ϣ.��ε�������ʹ�õĻ������Լ���������С.
		Result = SetupDiGetDeviceInterfaceDetail(
			hDevInfoSet,
			&DevInterfaceData,
			pDevDetailData,
			RequiredSize,
			NULL,
			NULL);
		//���豸·�����Ƴ���,Ȼ�����ٸո�������ڴ�.
		//MyDevPathName = pDevDetailData->DevicePath;
		//_tcscpy((char*)MyDevPathName,(char*)pDevDetailData->DevicePath );
		strcpy_s((char*)MyDevPathName,sizeof(MyDevPathName),(char*)pDevDetailData->DevicePath );
		free(pDevDetailData);
		//�������ʧ��,�������һ���豸.
		if(Result == FALSE) continue;
		//������óɹ�,��ʹ�ò�����д���ʵ�CreateFile����
		//����ȡ�豸������,����VID PID �汾�ŵ�.
		//����һЩ��ռ�豸(����usb����),ʹ�ö�д���ʷ��ʷ�ʽ���޷��򿪵�
		//��ʹ�ò�����д���ʵĸ�ʽ�ſ��Դ���Щ�豸,�Ӷ���ȡ�豸������.
		hDevHandle = CreateFile (MyDevPathName,
			NULL,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);	
		
		//����򿪳ɹ�,���ȡ�豸����
		if(hDevHandle != INVALID_HANDLE_VALUE)
		{
			//��ȡ�豸�����Բ�������DevAttributes�ṹ����
			Result = HidD_GetAttributes(hDevHandle,
				&DevAttributes);
			//�رոոմ򿪵��豸
			CloseHandle(hDevHandle);

			//��ȡʧ��,������һ��
			if(Result == FALSE) continue;

			//�����ȡ�ɹ�,�������е�VID/PId��������Ҫ��
			//���бȽ�,���һ�µĻ�,��˵������������Ҫ�ҵ��豸.
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
		//�����ʧ��,�������һ���豸
		else continue;
	}
	//����SetupDiDestroyDeviceInfoList���������豸��Ϣ����
	SetupDiDestroyDeviceInfoList(hDevInfoSet);

	//����豸�Ѿ��ҵ�,��ôӦ��ʹ�ܸ��ֲ�����ť,��ͬʱ��ֹ���豸��ť
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
	//��������CRC16У�飬��2���ֽ� �Ӱ��ſ�ʼ�����ݽ��������2�ֽ���CRC
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