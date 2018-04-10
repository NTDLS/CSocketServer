///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_ClientThread_Cpp
#define _CSocketServer_ClientThread_Cpp
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
#include <Process.H>

#include "CSocketServer.H"
#include "../../@Libraries/NSWFL/NSWFL.h"

using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to pause a clients thread.
*/
bool CSocketClient::PauseClientThread(void)
{
	return(SuspendThread(this->hClientThread) != -1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to resume a clients previously paused thread.
*/
bool CSocketClient::ResumeClientThread(void)
{
	return(ResumeThread(this->hClientThread) != -1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the handle of a clients thread.
*/
HANDLE CSocketClient::ThreadHandle(void)
{
	return this->hClientThread;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function starts a client thread, it should not be called directly.
*/
bool CSocketServer::p_StartClientThread(CSocketClient *pClient)
{
	//Are we supposed to create client threads?
	if (this->p_pClientHandlerThread)
	{

#ifdef UNICODE
		//FIXFIX: rand is not random enough, this needs to use a GUID.
		WCHAR sEventName[255];
		swprintf_s(sEventName, sizeof(sEventName) / sizeof(WCHAR),
			L"W->%d->%d->%d", pClient->PeerID(), GetTickCount(), rand());
		pClient->hThreadInitEvent = CreateEvent(NULL, FALSE, FALSE, sEventName);
#else
		char sEventName[255];
		NewGUID(sEventName, sizeof(sEventName));
		pClient->hThreadInitEvent = CreateEvent(NULL, FALSE, FALSE, sEventName);
#endif

		CLIENTTHREADSTARTUPINFO CTSI;
		memset(&CTSI, 0, sizeof(CTSI));
		CTSI.pSock = this;
		CTSI.pClient = pClient;
		CTSI.bIsNewThread = true;

		if (!CTSI.pClient)
		{
			__debugbreak();
		}

		if ((pClient->hClientThread = (HANDLE)
			_beginthreadex(NULL, 0, p_CilentThreadProc, (LPVOID)&CTSI, CREATE_SUSPENDED, NULL)) == NULL)
		{
			CloseHandle(pClient->hThreadInitEvent);
			CloseHandle(pClient->hDataReceived);
			CloseHandle(pClient->hDataSent);
			return this->p_ThrowError(12, pClient);
		}

		SetThreadPriority(pClient->hClientThread, THREAD_PRIORITY_NORMAL);

		ResumeThread(pClient->hClientThread);

		WaitForSingleObject(pClient->hThreadInitEvent, INFINITE);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is called to wait on client thread to end when the client is being disconnected.
	It should not be called directly.
*/
bool CSocketServer::p_WaitOnClientThreadToExit(CSocketClient *pClient)
{
	if (pClient->hClientThread) //Does the thread exist?
	{
		DWORD dwExitCode = 0;
		int iWaitCount = 0;
		bool bResult = false;

		if (pClient->hThreadInitEvent)
		{
			//The thread might be waiting on its hThreadInitEvent for some reason, signal it.
			SetEvent(pClient->hThreadInitEvent);
		}

		while ((bResult = (GetExitCodeThread(pClient->hClientThread, &dwExitCode) > 0)))
		{
			if (this->TryLock())
			{
				if (dwExitCode != STILL_ACTIVE || iWaitCount == SSTHREADTIMEOUT)
				{
					if (iWaitCount == SSTHREADTIMEOUT)
					{
						TerminateThread(pClient->hClientThread, 1);
						this->p_ThrowError(15, pClient);
						bResult = false;
					}

					//The thread was stopped, trigger the event.
					if (this->pOnEndClientThread)
					{
						this->pOnEndClientThread(this, pClient);
					}

					this->Unlock();
					break;
				}
				else {
					if (iWaitCount == 0)
					{
						//If the thread is still active, then we'll give it the
						//	priority it needs to finish up under a high load.
						SetThreadPriority(pClient->hClientThread, THREAD_PRIORITY_ABOVE_NORMAL);
					}
				}
				iWaitCount++;
				this->Unlock();
			}
			Sleep(10);
		}

		if (!bResult)
		{
			this->p_ThrowError(14, pClient);
		}

		//The thread was stopped, trigger the event.
		if (this->pOnEndClientThread)
		{
			this->pOnEndClientThread(this, pClient);
		}

		CloseHandle(pClient->hClientThread);
		CloseHandle(pClient->hThreadInitEvent);
		CloseHandle(pClient->hDataReceived);
		CloseHandle(pClient->hDataSent);

		pClient->hClientThread = NULL;
		pClient->hThreadInitEvent = NULL;
		pClient->hDataReceived = NULL;
		pClient->hDataSent = NULL;

		return bResult;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This is the actual client thread procedure.
*/
unsigned __stdcall p_CilentThreadProc(void *pData)
{
	//---------------------(Initialize Thread [Begin])----------
	CLIENTTHREADSTARTUPINFO *CTSI = (CLIENTTHREADSTARTUPINFO *)pData;
	CSocketServer *pSock = CTSI->pSock;
	CSocketClient *pClient = CTSI->pClient;
	bool bIsNewThread = CTSI->bIsNewThread;

	if (!pClient)
	{
		__debugbreak();
	}

#ifdef _DEBUG
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	char sName[255];
	sprintf_s(sName, sizeof(sName), "Peer %d", pClient->PeerID());
	pSock->p_SetThreadName(sName);
#endif
#endif

	SetEvent(pClient->hThreadInitEvent);
	//---------------------(Initialize Thread [END])----------

	//The thread has started, trigger the event.
	if (pSock->pOnBeginClientThread)
	{
		pSock->pOnBeginClientThread(pSock, pClient, pClient->hClientThread);
	}

	BASICHUNK pChunk;
	if (pSock->Alloc(&pChunk))
	{
		pSock->p_pClientHandlerThread(pSock, pClient, &pChunk);
		pSock->Free(&pChunk);
	}
	else {
		pSock->p_ThrowError(13, pClient);
	}

	if (pClient->Reserved())
	{
		pClient->Reserved(false);
	}

	if (pClient->IsConnected())
	{
		pClient->Disconnect();
	}

	if (bIsNewThread)
	{
		_endthreadex(0);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
