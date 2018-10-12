#pragma once
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the HIDSDK_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// HIDSDK_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#include <windows.h>

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <dbt.h>
#include <string.h>

extern "C"{
#include "setupapi.h"
#include "hidsdi.h"
}

#pragma comment (lib,"hid.lib")
#pragma comment (lib,"Setupapi.lib")

#ifdef HIDSDK_EXPORTS
#define HIDSDK_API __declspec(dllexport)
#else
#define HIDSDK_API __declspec(dllimport)
#endif

// This class is exported from the HidSdk.dll
class CHidSdk {
public:
	CHidSdk(void);
	// TODO: add your methods here.
};


#define HID_MAX_PACKET_SIZE_EP 64

class CHidIO {
	
protected:
	HANDLE m_hReadHandle;
	HANDLE m_hWriteHandle;
	HANDLE m_hReadEvent;
	HANDLE m_hWriteEvent;
	HANDLE m_hAbordEvent;
	BOOL m_bUseTwoHandle;

public:
	CHidIO();
	~CHidIO();
	void CloseDevice();
	BOOL OpenDevice(BOOL bUseTwoHandle, USHORT usVID,USHORT usPID);
	BOOL ReadFile(char *pcBuffer, size_t szMaxLen, DWORD *pdwLength, DWORD dwMilliseconds);
	BOOL WriteFile( char *pcBuffer, size_t szLen, DWORD *pdwLength, DWORD dwMilliseconds);
};

class CHidCmd {

protected:
	CHAR	m_acBuffer[HID_MAX_PACKET_SIZE_EP + 1];
	UCHAR	m_ucCmdIndex;
	BOOL	m_bCmdError;
	CHidIO	m_hidIO;
public:
	 CHidCmd();
	 ~CHidCmd();
	 void CloseDevice();
	 BOOL OpenDevice(USHORT usVID, USHORT usPID);
	 BOOL ReadFile(char *pcBuffer,size_t szMaxLen,DWORD *pdwLength,DWORD dwMilliseconds);
	 BOOL WriteFile( char *pcBuffer ,DWORD dwLen ,DWORD *pdwLength ,DWORD dwMilliseconds);
	 BOOL IsCmdError();
	 USHORT CRC16(CHAR *puchMsgg,DWORD usDataLen,USHORT crcInput);
};