///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_Internal_Cpp
#define _CSocketServer_Internal_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WinSock2.H>
#include <WinSock.H>
#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Time.H>

#include "CSocketServer.H"

#include "../NSWFL/NSWFL.h"

using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function sets up a socket to listen for incomming connections.
	This function should not be called directly!
*/
bool CSocketServer::p_OpenListenSocket(int iListenPort)
{
	for (int iIP = 0; iIP < this->p_ListenAddrs.Count || this->p_ListenAddrs.Count == 0; iIP++)
	{
		SOCKADDR_IN LocalListenSocketAddress; // Socket address of local server

		// Fill In The Address Structure For Local Server
		LocalListenSocketAddress.sin_family = AF_INET; // Address Family.
		LocalListenSocketAddress.sin_port = htons(iListenPort); // Port Number.

		if (this->p_ListenAddrs.Count > 0)
		{
			LocalListenSocketAddress.sin_addr.s_addr = inet_addr(this->p_ListenAddrs.Address[iIP].IP);
		}
		else {
			LocalListenSocketAddress.sin_addr.s_addr = INADDR_ANY; // Let socket Library Assign Address.
		}

		int iMemory = (this->p_ListenSockets.Count + 1) * sizeof(SOCKET);
		this->p_ListenSockets.Socket = (SOCKET *) this->ReAlloc(this->p_ListenSockets.Socket, iMemory);
		SOCKET *pSocket = &this->p_ListenSockets.Socket[this->p_ListenSockets.Count++];

		// Create A TCP/IP Stream Socket To "Listen" with.
		if ((*pSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		{
			return this->p_ThrowError(16);
		}

		if (this->p_bForceSocketReuse)
		{
			int iOptVal = 1;
			if (setsockopt((SOCKET)*pSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&iOptVal, sizeof(int)) == -1)
			{
				closesocket(*pSocket);
				return this->p_ThrowError(34);
			}
		}

		//Turn-on non-Blocking mode, otherwise accept() can block.
		DWORD dwNonBlockingMode = 1;
		ioctlsocket((SOCKET)*pSocket, FIONBIO, &dwNonBlockingMode);

		// Bind The Name To The Socket.
		if (bind(*pSocket, (SOCKADDR*)&LocalListenSocketAddress, sizeof(struct sockaddr)) == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			closesocket(*pSocket);
			return this->p_ThrowError(17);
		}

		// Set The Socket To Listen.
		if (listen(*pSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			closesocket(*pSocket);
			return this->p_ThrowError(18);
		}

		if (this->p_ListenAddrs.Count == 0)
		{
			break; //We are listening on all addresses.
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function closes down the listening socket.
	This function should not be called directly!
*/
bool CSocketServer::p_CloseListenSocket(void)
{
	bool bResult = true;

	for (int iSocket = 0; iSocket < this->p_ListenSockets.Count; iSocket++)
	{
		if (closesocket(this->p_ListenSockets.Socket[iSocket]) == SOCKET_ERROR)
		{
			bResult = false;
		}
	}

	if (this->p_ListenSockets.Socket)
	{
		this->Free(this->p_ListenSockets.Socket);
		this->p_ListenSockets.Socket = NULL;
	}

	this->p_ListenSockets.Count = 0;

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is called to disconnect a client.
	This functions return value has no meaning.
	This function should not be called directly!
*/
void CSocketServer::p_DisconnectClient(CSocketClient *pClient)
{
	//%%___|JMP 2012/06/10
	//If a client thread process is running long and the client has
	//	disconnected, then this would block all other connections.
	//	Perhaps we should wait of the client thread to complete for
	//	a few ms then continue and try to wait again later.

	this->p_WaitOnClientThreadToExit(pClient);

	this->Lock(); //We MUST lock here, this cannot be a TryLock.

	//The client is not connected.
	if (!pClient->bConnected)
	{
		pClient->bDisconnect = false;
		return;
	}

	pClient->bConnected = false;
	pClient->bDisconnect = false;
	pClient->bAbortConnection = false;

	this->p_iCurrentClients--;

	if (this->p_iCurrentClients == 0)
	{
		//Do a bit of house cleaning on the internal shared buffers.
		if (this->p_Buffer.iAlloc > this->p_iInitialBufferSize)
		{
			this->ReAlloc(&this->p_Buffer, this->p_iInitialBufferSize);
		}
	}

	shutdown(pClient->Socket, SD_BOTH);
	closesocket(pClient->Socket);

	if (this->pOnDisconnect)
	{
		this->pOnDisconnect(this, pClient);
	}

	this->Free(&pClient->SendChunk);
	this->Free(&pClient->RecvChunk);

	this->p_ResetClientSlot(pClient);

	this->Unlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is called for each connecting client just
	before the client's bcConnected flag is set to TRUE.

	If this function returns FALSE the client will not be accepted
	or connected. This function should not be called directly!
*/
bool CSocketServer::p_InitializeClientVariables(CSocketClient *pClient)
{
	if (this->pOnAcceptConnect)
	{
		if (!this->pOnAcceptConnect(this, pClient))
		{
			return false;
		}
	}

	//Allocate RAM for the clients send buffer.
	if (!this->Alloc(&pClient->SendChunk, this->p_iInitialBufferSize))
	{
		return false;
	}

	//Allocate RAM for the clients receive buffer.
	if (!this->Alloc(&pClient->RecvChunk, this->p_iInitialBufferSize))
	{
		return false;
	}

#ifdef UNICODE
	//FIXFIX: rand is not random enough, this needs to use a GUID.
	WCHAR sEventName[255];
	swprintf_s(sEventName, sizeof(sEventName) / sizeof(WCHAR),
		L"R->%d->%d->%d", pClient->PeerID(), GetTickCount(), rand());
	pClient->hDataReceived = CreateEvent(NULL, FALSE, FALSE, sEventName);
	swprintf_s(sEventName, sizeof(sEventName) / sizeof(WCHAR),
		L"S->%d->%d->%d", pClient->PeerID(), GetTickCount(), rand());
	pClient->hDataSent = CreateEvent(NULL, FALSE, FALSE, sEventName);
#else
	char sEventName[255];
	NewGUID(sEventName, sizeof(sEventName));
	pClient->hDataReceived = CreateEvent(NULL, FALSE, FALSE, sEventName);
	NewGUID(sEventName, sizeof(sEventName));
	pClient->hDataSent = CreateEvent(NULL, FALSE, FALSE, sEventName);
#endif

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the passed in error number is OK, otherwise this function
		will call the OnError event then return false.
	This function should not be called directly!
*/
bool CSocketServer::p_ThrowError(int iErrorNumber, CSocketClient *pClient)
{
	if (!iErrorNumber)
	{
		return true;
	}
	else {
		if (this->pOnError)
		{
			this->pOnError(this, pClient, iErrorNumber, sSocketServerError[iErrorNumber]);
		}
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the passed in error number is OK, otherwise this function
		will call the OnError event then return false.
	This function should not be called directly!
*/
bool CSocketServer::p_ThrowError(int iErrorNumber)
{
	return this->p_ThrowError(iErrorNumber, NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function searched the bcConnected[] array to find a free client slot.

	If this function returns -1 the client wll not be accepted or connected.
	This function should not be called directly!
*/
CSocketClient *CSocketServer::p_GetFreeSocketSlot(int *pOutSlot)
{
	for (int iSlot = 0; iSlot < this->p_iMaxConnections; iSlot++)
	{
		if (!this->p_Clients[iSlot].bConnected && !this->p_Clients[iSlot].bDisconnect && !this->p_Clients[iSlot].bReserved)
		{
			this->p_ResetClientSlot(&this->p_Clients[iSlot]);
			*pOutSlot = iSlot;
			return &this->p_Clients[iSlot];
		}
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to clear the Clients position in the CSocketClient structure.
	The memset() is strictly for security purposes.

	This function should not be called directly!
*/
void CSocketServer::p_ResetClientSlot(CSocketClient *pClient)
{
	LPVOID lpUserData = pClient->pUserData;
	memset(pClient, 0, sizeof(CSocketClient));
	pClient->pUserData = lpUserData;
	pClient->bPacketMode = this->p_bPacketMode;
	pClient->pSockServer = this;
	pClient->bAbortConnection = false;
	time(&pClient->ttLastActivity);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
void CSocketServer::p_SetThreadName(const char *sThreadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = sThreadName;
	info.dwThreadID = GetCurrentThreadId();
	info.dwFlags = 0;

	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}
#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
