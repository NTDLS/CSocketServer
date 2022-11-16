///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_Utilities_Cpp
#define _CSocketServer_Utilities_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN 
#define _WINSOCK_DEPRECATED_NO_WARNINGS

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WinSock2.H>
#include <WinSock.H>
#include <Objbase.h>
#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Time.H>
#include <Ws2tcpip.h>

#include "CSocketServer.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function resolves a Host Name to an IPv4 IP Address.
*/
#ifdef UNICODE
bool CSocketServer::ResolveHost(const WCHAR *wsHostName, WCHAR *sOutIP, int iMaxOutSz)
#else
bool CSocketServer::ResolveHost(const char *sHostName, char *sOutIP, int iMaxOutSz)
#endif
{
	EnterCriticalSection(&this->p_HostResolverLock);

#ifdef UNICODE
	size_t iConverted = 0;
	char sHostName[1024];
	wcstombs_s(&iConverted, sHostName, sizeof(sHostName), wsHostName, wcslen(wsHostName) * sizeof(WCHAR));
#endif

	struct in_addr Address;
	struct hostent *Host;

	memset(&Host, 0, sizeof(Host));
	memset(&Address, 0, sizeof(Address));

	if ((Host = gethostbyname(sHostName)) == NULL)
	{
		LeaveCriticalSection(&this->p_HostResolverLock);
		return this->p_ThrowError(30);
	}

	memcpy(&Address.s_addr, (*Host).h_addr_list[0], (*Host).h_length);

#ifdef UNICODE
	iConverted = 0;
	char *sAddress = inet_ntoa(Address);
	mbstowcs_s(&iConverted, sOutIP, iMaxOutSz / sizeof(WCHAR), sAddress, strlen(sAddress));
#else
	strcpy_s(sOutIP, iMaxOutSz, inet_ntoa(Address));
#endif

	LeaveCriticalSection(&this->p_HostResolverLock);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the passed in address is a valid IPv4 IP address.
*/
#ifdef UNICODE
bool CSocketServer::IsIPAddress(const WCHAR *wsString)
#else
bool CSocketServer::IsIPAddress(const char *sString)
#endif
{
#ifdef UNICODE
	size_t iConverted = 0;
	char sString[1024];
	wcstombs_s(&iConverted, sString, sizeof(sString), wsString, wcslen(wsString) * sizeof(WCHAR));
#endif

	int iStrSz = (int)strlen(sString);

	int iDotCount = 0;
	char sSeg[5];

	int iWPos = 0;

	if (iStrSz < 7 || iStrSz > 15 || inet_addr(sString) == INADDR_NONE)
	{
		return false;
	}
	else {
		for (int iRPos = 0; iRPos < iStrSz; iRPos++)
		{
			sSeg[iWPos++] = sString[iRPos];

			if (sString[iRPos] == '.')
			{
				int iVal = atol(sSeg);
				sSeg[iWPos] = '\0';

				if (iVal < 0 || iVal > 255)
				{
					return false;
				}

				iWPos = 0;
				iDotCount++;
			}

			if (iWPos > 3)
			{
				return false;
			}
		}
	}

	return iDotCount == 3;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef UNICODE
bool CSocketServer::NewGUID(WCHAR* sOut, int iMaxOutSz)
#else
bool CSocketServer::NewGUID(char* sOut, int iMaxOutSz)
#endif
{
	GUID lGUID;
	memset(&lGUID, 0, sizeof(GUID));
	if (CoCreateGuid(&lGUID) == S_OK)
	{
		if (Bin2Hex(sOut, iMaxOutSz, &lGUID, sizeof(GUID), 2, false) == 32)
		{
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CSocketServer::Bin2Hex(void* lpTarget, int iMaxSz, const void* lpSrc, int iSz, int iBytes, bool bPrefix)
{
	int iWPos = 0;
	int iRPos = 0;
	int iByte = 0;

	if (iBytes < 1)
	{
		return 0;
	}

	if (bPrefix)
	{
		((char*)lpTarget)[iWPos++] = '0';
		((char*)lpTarget)[iWPos++] = 'x';
	}

	while (iRPos < iSz)
	{
		iByte = (iBytes - 1);

		while (iByte >= 0)
		{
			((char*)lpTarget)[iWPos++] = "0123456789ABCDEF"[((((char*)lpSrc)[iRPos] >> (iByte--) * 4) & 0xF)];
		}
		iRPos++;
	}

	((char*)lpTarget)[iWPos] = '\0';

	return iWPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CSocketServer::Bin2Hex(void* lpTarget, int iMaxSz, const void* lpSrc, int iSz, int iBytes)
{
	return Bin2Hex(lpTarget, iMaxSz, lpSrc, iSz, iBytes, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
