///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_Command_Cpp
#define _CSocketServer_Command_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WinSock2.H>
#include <WinSock.H>
#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Time.H>

#include "CSocketServer.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Checks to see if the passed in command data (lpInCmd) is the specified command tag (sCommand),
		if it matches, this function will parse off the data section of the lpInCmd buffer and
		place it in the return command (lpOutCmd).

	If ParseCommand() returns true, you must call CSocketServer::Free() to free the allocated block.
*/
#ifdef UNICODE
bool CSocketServer::ParseCommand(LPBASICHUNK lpInCmd, const WCHAR *sCommand, LPBASICHUNK lpOutCmd)
#else
bool CSocketServer::ParseCommand(LPBASICHUNK lpInCmd, const char *sCommand, LPBASICHUNK lpOutCmd)
#endif

{
	int iCommandLength = 0;

#ifdef UNICODE
	if ((iCommandLength = CompareCommand((WCHAR *)lpInCmd->pBuffer, lpInCmd->iLength, sCommand)))
#else
	if ((iCommandLength = CompareCommand((char *)lpInCmd->pBuffer, lpInCmd->iLength, sCommand)))
#endif
	{
		int iMemoryRequired = (lpInCmd->iLength - iCommandLength);
		if (iMemoryRequired > 0)
		{
			if (iMemoryRequired > lpOutCmd->iAlloc)
			{
				if (lpOutCmd->iAlloc <= 0)
				{
					lpOutCmd->pBuffer = NULL;
				}

				lpOutCmd->iAlloc = iMemoryRequired + 1;

				if (!(lpOutCmd->pBuffer = (char *) this->ReAlloc(lpOutCmd->pBuffer, lpOutCmd->iAlloc)))
				{
					return false;
				}
			}

			lpOutCmd->iLength = BreakDataFromCommand(lpInCmd->pBuffer,
				lpInCmd->iLength, iCommandLength, lpOutCmd->pBuffer, lpOutCmd->iAlloc);

			((BYTE *)lpInCmd->pBuffer)[lpOutCmd->iLength] = '\0';

			return true;
		}
		else {
			lpOutCmd->iAlloc = 0;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef UNICODE
bool CSocketServer::IsCommand(LPBASICHUNK lpInCmd, const WCHAR *sCommand)
{
	return(this->CompareCommand((WCHAR *)lpInCmd->pBuffer, lpInCmd->iLength, sCommand) > 0);
}
#else
bool CSocketServer::IsCommand(LPBASICHUNK lpInCmd, const char *sCommand)
{
	return(this->CompareCommand((char *)lpInCmd->pBuffer, lpInCmd->iLength, sCommand) > 0);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef UNICODE
bool CSocketServer::ParseCommand(LPBASICHUNK lpInOutCmd, const WCHAR *sCommand)
#else
bool CSocketServer::ParseCommand(LPBASICHUNK lpInOutCmd, const char *sCommand)
#endif
{
	return this->ParseCommand(lpInOutCmd, sCommand, lpInOutCmd);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Checks to see if the passed in buffer starts with the specified command tag (sCommand).

	The function will return the length of the sCommand string.
*/
#ifdef UNICODE
int CSocketServer::CompareCommand(LPBASICHUNK lpInOutCmd, const WCHAR *sCommand)
{
	int iCommandLength = (int)wcslen(sCommand) * sizeof(WCHAR);

	if (iCommandLength <= lpInOutCmd->iLength)
	{
		if (_wcsnicmp((WCHAR *)lpInOutCmd->pBuffer, sCommand, iCommandLength) == 0)
		{
			return iCommandLength;
		}
	}
	return 0;
}
#else
int CSocketServer::CompareCommand(LPBASICHUNK lpInOutCmd, const char *sCommand)
{
	int iCommandLength = (int)strlen(sCommand);

	if (iCommandLength <= lpInOutCmd->iLength)
	{
		if (_strnicmp((char *)lpInOutCmd->pBuffer, sCommand, iCommandLength) == 0)
		{
			return iCommandLength;
		}
	}
	return 0;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Checks to see if the passed in buffer (sBuffer) starts with the specified command tag (sCommand).

	The function will return the length of the sCommand string.
*/
#ifdef UNICODE
int CSocketServer::CompareCommand(const WCHAR *sBuffer, const int iBufferSize, const WCHAR *sCommand)
{
	int iCommandLength = (int)wcslen(sCommand) * sizeof(WCHAR);
	if (iCommandLength <= iBufferSize)
	{
		if (_wcsnicmp(sBuffer, sCommand, iCommandLength) == 0)
		{
			return iCommandLength;
		}
	}
	return 0;
}
#else
int CSocketServer::CompareCommand(const char *sBuffer, const int iBufferSize, const char *sCommand)
{
	int iCommandLength = (int)strlen(sCommand);

	if (iCommandLength <= iBufferSize)
	{
		if (_strnicmp(sBuffer, sCommand, iCommandLength) == 0)
		{
			return iCommandLength;
		}
	}
	return 0;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Parses off the data section of a command (lpInCmd) and places it in the return buffer (lpOutData).

	lpOutCmd must be freed externally.
*/
#ifdef UNICODE
int CSocketServer::BreakDataFromCommand(LPBASICHUNK lpInCmd, const int iCommandLength, LPBASICHUNK lpOutData)
{
	int iDataLength = lpInCmd->iLength - iCommandLength;

	if (iDataLength >= 0)
	{
		this->Alloc(lpOutData, iDataLength + 1);
		memcpy_s(lpOutData->pBuffer, lpOutData->iAlloc, ((WCHAR *)lpInCmd->pBuffer) + iCommandLength, iDataLength);
		lpOutData->iLength = iDataLength;
		return iDataLength;
	}

	return 0;
}
#else
int CSocketServer::BreakDataFromCommand(LPBASICHUNK lpInCmd, const int iCommandLength, LPBASICHUNK lpOutData)
{
	int iDataLength = lpInCmd->iLength - iCommandLength;

	if (iDataLength >= 0)
	{
		this->Alloc(lpOutData, iDataLength + 1);
		memcpy_s(lpOutData->pBuffer, lpOutData->iAlloc, ((char *)lpInCmd->pBuffer) + iCommandLength, iDataLength);
		lpOutData->iLength = iDataLength;
		return iDataLength;
	}

	return 0;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Parses off the data section of a command (in pBuffer) and places it in the return buffer (sOutBuffer).

	Returns the length of the data section of the command.
*/
int CSocketServer::BreakDataFromCommand(const void *pBuffer, const int iBufferLength,
	const int iCommandLength, void *sOutBuffer, int iMaxOutBufferSz)
{
	memcpy_s(sOutBuffer, iMaxOutBufferSz - iCommandLength,
		((BYTE *)pBuffer) + iCommandLength, iBufferLength - iCommandLength);
	return iBufferLength - iCommandLength;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Compares the input command (sCommand) to the command in the input buffer (lpInCmd). If they match this function will
		parse the data off the end of the command and return it in the output buffer (lpOutData).

	If this function returns true, you must free the data allocated to the output buffer (lpOutData).
*/
#ifdef UNICODE
bool CSocketServer::CompareCommandAndBreakData(LPBASICHUNK lpInCmd, const WCHAR *sCommand, LPBASICHUNK lpOutData)
{
	int iCmdLen = 0;

	if ((iCmdLen = this->CompareCommand(lpInCmd, sCommand)) > 0)
	{
		this->BreakDataFromCommand(lpInCmd, iCmdLen, lpOutData);
		return true;
	}

	return false;
}
#else
bool CSocketServer::CompareCommandAndBreakData(LPBASICHUNK lpInCmd, const char *sCommand, LPBASICHUNK lpOutData)
{
	int iCmdLen = 0;

	if ((iCmdLen = this->CompareCommand(lpInCmd, sCommand)) > 0)
	{
		this->BreakDataFromCommand(lpInCmd, iCmdLen, lpOutData);
		return true;
	}

	return false;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Sends a textual command.

	See SetNextSendData() for return values.
*/
#ifdef UNICODE
bool CSocketClient::SendCommand(const WCHAR *sCommand)
{
	return this->SetNextSendData(sCommand, (int)wcslen(sCommand) * sizeof(WCHAR));
}
#else
bool CSocketClient::SendCommand(const char *sCommand)
{
	return this->SetNextSendData(sCommand, (int)strlen(sCommand));
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Appends a textual command to command data and send the chunk.

	See SetNextSendData() for return values.
*/
#ifdef UNICODE
bool CSocketClient::SendCommand(const WCHAR *sCommand, const WCHAR *sData)
{
	return this->SendCommand(sCommand, sData, (int)wcslen(sData) * sizeof(WCHAR));
}
#else
bool CSocketClient::SendCommand(const char *sCommand, const char *sData)
{
	return this->SendCommand(sCommand, sData, (int)strlen(sData));
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Appends a textual command to command data and send the chunk.

	pTemp must be large enough to hold both sCommand and pData.

	See SetNextSendData() for return values.
*/
#ifdef UNICODE
bool CSocketClient::SendCommand(const WCHAR *sCommand, const void *pData, const int iDataLength)
#else
bool CSocketClient::SendCommand(const char *sCommand, const void *pData, const int iDataLength)
#endif
{
	BASICHUNK Temp;

	memset(&Temp, 0, sizeof(Temp));
	bool bResult = this->SendCommand(sCommand, pData, iDataLength, &Temp);
	this->Free(&Temp);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Appends a textual command to command data and send the chunk.

	See SetNextSendData() for return values.
*/
#ifdef UNICODE
bool CSocketClient::SendCommand(const WCHAR *sCommand,
	const void *pData, const int iDataLength,
	BASICHUNK *pTempChunk)
#else
bool CSocketClient::SendCommand(const char *sCommand,
	const void *pData, const int iDataLength,
	BASICHUNK *pTempChunk)
#endif
{
#ifdef UNICODE
	int iCommandLength = (int)wcslen(sCommand) * sizeof(WCHAR);
#else
	int iCommandLength = (int)strlen(sCommand);
#endif

	pTempChunk->iLength = iCommandLength + iDataLength;

	if (pTempChunk->iAlloc <= pTempChunk->iLength)
	{
		if (!this->ReAlloc(pTempChunk, pTempChunk->iLength + 1))
		{
			pTempChunk->iLength = 0;
			return false;
		}
	}

	memcpy_s(pTempChunk->pBuffer, pTempChunk->iAlloc, sCommand, iCommandLength);

	memcpy_s(((BYTE *)pTempChunk->pBuffer) + iCommandLength,
		pTempChunk->iAlloc - iCommandLength, pData, iDataLength);

	((BYTE *)pTempChunk->pBuffer)[pTempChunk->iLength] = '\0';

	return this->SetNextSendData(pTempChunk);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
