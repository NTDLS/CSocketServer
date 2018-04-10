///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_PumpThread_Cpp
#define _CSocketServer_PumpThread_Cpp
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
	This function is used to pause the data pump thread.
*/
bool CSocketServer::PausePumpThread(void)
{
	return(SuspendThread(this->p_hTcpPump) != -1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to resume the previously paused data pump thread.
*/
bool CSocketServer::ResumePumpThread(void)
{
	return(ResumeThread(this->p_hTcpPump) != -1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to start the data pump thread. It should not be called directly.
*/
bool CSocketServer::p_StartTcpPump(void)
{
#ifdef UNICODE
	WCHAR sEventName[255];
	swprintf_s(sEventName, sizeof(sEventName) / sizeof(WCHAR),
		L"TP->%d->%d", GetTickCount(), rand());
#else
	char sEventName[255];
	//sprintf_s(sEventName, sizeof(sEventName), "TP->%d->%d", GetTickCount(), rand());
	NewGUID(sEventName, sizeof(sEventName));
#endif

	this->p_hTcpPumpEvent = CreateEvent(NULL, TRUE, FALSE, sEventName);

	if ((this->p_hTcpPump = (HANDLE)_beginthreadex(NULL, 0,
		p_TcpPumpThreadProc, (LPVOID) this, CREATE_SUSPENDED, NULL)) == NULL)
	{
		CloseHandle(p_hTcpPumpEvent);
		return this->p_ThrowError(23);
	}

	SetThreadPriority(this->p_hTcpPump, THREAD_PRIORITY_ABOVE_NORMAL);

	ResumeThread(this->p_hTcpPump);

	WaitForSingleObject(p_hTcpPumpEvent, INFINITE);
	CloseHandle(p_hTcpPumpEvent);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to stop the data pump thread. It should not be called directly.
*/
bool CSocketServer::p_StopTcpPump(void)
{
	DWORD dwExitCode = 0;

	if (this->p_bTcpPumpActive)
	{
		this->p_bShutdownTcpPump = true;

		while (GetExitCodeThread(this->p_hTcpPump, &dwExitCode))
		{
			if (dwExitCode != STILL_ACTIVE)
			{
				CloseHandle(this->p_hTcpPump);
				return true;
			}
			Sleep(1);
		}

		return this->p_ThrowError(24);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This is the actual data pump procedure,
		it is called by the data pump thread procedure: p_TcpPumpThreadProc().
*/
unsigned int CSocketServer::p_TCPPump(void)
{
	int iSelectResult = 0;

	struct timeval tTimeOut; // The TimeOut structure.
	tTimeOut.tv_sec = 1;
	tTimeOut.tv_usec = 0;

	fd_set fdExcept;
	fd_set fdRead;
	fd_set fdWrite;

	time_t ttNow;

	bool bIncrementSpinCount = false;

	int iSpinCount = 0;

	while (!this->p_bShutdownTcpPump)
	{
		bIncrementSpinCount = false;

		int iSlot = 0;

		// Zero the Read, Write and Except
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExcept);

		if (!this->p_bIsListeningServer)
		{
			while (this->p_iCurrentClients == 0)
			{
				if (this->p_bShutdownTcpPump)
				{
					return true;
				}
				Sleep(1);
			}
			if (this->p_bShutdownTcpPump)
			{
				return true;
			}
		}
		else {
			// Set LocalListenSocket socket to Read, Except

			for (int iSocket = 0; iSocket < this->p_ListenSockets.Count; iSocket++)
			{
				FD_SET(this->p_ListenSockets.Socket[iSocket], &fdRead);
				FD_SET(this->p_ListenSockets.Socket[iSocket], &fdExcept);
			}
		}

		// Set all connected clients to Read, Write and Except
		if (this->p_iCurrentClients > 0)
		{
			for (iSlot = 0; iSlot < this->p_iMaxConnections; iSlot++)
			{
				if (this->p_Clients[iSlot].bConnected)
				{
					FD_SET(this->p_Clients[iSlot].Socket, &fdRead);
					FD_SET(this->p_Clients[iSlot].Socket, &fdWrite);
					FD_SET(this->p_Clients[iSlot].Socket, &fdExcept);
				}
			}
		}

		// Socket Select...
		if (iSelectResult = select(-1, &fdRead, &fdWrite, &fdExcept, &tTimeOut) == SOCKET_ERROR)
		{
			return this->p_ThrowError(25);
		}

		if (this->p_bIsListeningServer && this->p_bAllowNewConnections)
		{
			// Accept any incoming connections.
			for (int iSocket = 0; iSocket < this->p_ListenSockets.Count; iSocket++)
			{
				if (FD_ISSET(this->p_ListenSockets.Socket[iSocket], &fdRead))
				{
					//Try to lock because new connections are not as
					//	important as the performance of existing connections.
					if (this->TryLock())
					{
						this->p_Accept(this->p_ListenSockets.Socket[iSocket]);
						this->Unlock();
						bIncrementSpinCount = true;
					}
				}
			}
		}

		// Send and receive any pending data.
		if (this->p_iCurrentClients > 0)
		{
			for (iSlot = 0; iSlot < this->p_iMaxConnections; iSlot++)
			{
				if (this->p_Clients[iSlot].bConnected)
				{
					if (this->TryLock())
					{
						/*
							Send, receive then test for errors.

							Since we almost always send a response [in response] to received data,
								we want to perform the receive first, followed immediately by a send.
						*/

						if (FD_ISSET(this->p_Clients[iSlot].Socket, &fdRead))
						{
							if (this->p_Receive(&this->p_Clients[iSlot]))
							{
								bIncrementSpinCount = true;
							}
						}

						if (FD_ISSET(this->p_Clients[iSlot].Socket, &fdWrite))
						{
							if (this->p_Send(&this->p_Clients[iSlot]))
							{
								bIncrementSpinCount = true;
							}
						}

						if (FD_ISSET(this->p_Clients[iSlot].Socket, &fdExcept))
						{
							if (this->p_Clients[iSlot].Disconnect())
							{
								bIncrementSpinCount = true;
							}
						}

						if (this->p_dMaxInactivityTime > 0) //Is max inactivity time enabled?
						{
							if (!this->p_Clients[iSlot].bReserved) //Don't disconnect clients that have been reserved.
							{
								//Has the client been inactive for too long (no send/receive for x seconds)?
								if (difftime(time(&ttNow), this->p_Clients[iSlot].ttLastActivity) > this->p_dMaxInactivityTime)
								{
									this->p_Clients[iSlot].Disconnect();
									this->p_ThrowError(2, &this->p_Clients[iSlot]);
								}
							}
						}

						if (this->pOnClientPump)
						{
							this->pOnClientPump(this, &this->p_Clients[iSlot]);
						}

						this->Unlock();
					}

					// Disconnect any pending disconnects.
					if (this->p_Clients[iSlot].bDisconnect && !this->p_Clients[iSlot].bReserved)
					{
						this->p_DisconnectClient(&this->p_Clients[iSlot]);
					}
				}
			}
		}

		if (this->p_bOptimizeForThroughput)
		{
			if (bIncrementSpinCount)
			{
				if (iSpinCount < SSWAITSPINCOUNT)
				{
					iSpinCount += 100;
				}
			}
			else {
				if (iSpinCount > 0)
				{
					iSpinCount--;
				}
				else {
					Sleep(1);
				}
			}
		}
		else {
			if (!bIncrementSpinCount)
			{
				Sleep(1);
			}
		}

		if (this->pOnPump)
		{
			this->pOnPump(this);
		}
	}

	this->p_bTcpPumpActive = false;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This is the actual data pump thread.
*/
unsigned __stdcall p_TcpPumpThreadProc(void *pData)
{
	//---------------------(Initialize Thread [Begin])----------
	CSocketServer* pSock = (CSocketServer *)pData;

	pSock->p_bShutdownTcpPump = false;
	pSock->p_bTcpPumpActive = true;

#ifdef _DEBUG
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	pSock->p_SetThreadName("TCPPump");
#endif
#endif

	SetEvent(pSock->p_hTcpPumpEvent);
	//---------------------(Initialize Thread [END])----------

#ifdef UNICODE
	WCHAR sEventName[255];
	swprintf_s(sEventName, sizeof(sEventName) / sizeof(WCHAR),
		L"PumpCycle->%d->%d", GetTickCount(), rand());
#else
	char sEventName[255];
	//sprintf_s(sEventName, sizeof(sEventName), "PumpCycled->%d->%d", GetTickCount(), rand());
	NewGUID(sEventName, sizeof(sEventName));
#endif
	pSock->p_PumpCycledEvent = CreateEvent(NULL, FALSE, FALSE, sEventName);

#ifdef UNICODE
	swprintf_s(sEventName, sizeof(sEventName) / sizeof(WCHAR),
		L"PumpCycle->%d->%d", GetTickCount(), rand());
#else
	//sprintf_s(sEventName, sizeof(sEventName), "CyclePump->%d->%d", GetTickCount(), rand());
	NewGUID(sEventName, sizeof(sEventName));
#endif

	_endthreadex(pSock->p_TCPPump());
	CloseHandle(pSock->p_PumpCycledEvent);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
