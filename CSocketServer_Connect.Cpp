///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_Connect_Cpp
#define _CSocketServer_Connect_Cpp
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
#include "../../@Libraries/NSWFL/NSWFL.h"

using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Establishes a new connection to a remote peer.

	This Function is not multi-thread safe, do not use this function directly.
*/
#ifdef UNICODE
bool CSocketServer::p_Connect(const WCHAR *wsIPAddress, int iRemotePort, CSocketClient **pOutClient,
	bool bCreateThread, bool bUseCurrentThread)
#else
bool CSocketServer::p_Connect(const char *sIPAddress, int iRemotePort, CSocketClient **pOutClient,
	bool bCreateThread, bool bUseCurrentThread)
#endif
{
	if (!this->p_bAllowNewConnections)
	{
		return false;
	}

	//Try to aquire the lock.
	for (int iRetry = 0; !this->TryLock(); iRetry++)
	{
		if (iRetry == 5000)
		{
			return false;
		}
		Sleep(1);
	}

#ifdef UNICODE
	size_t iConverted = 0;
	char sIPAddress[1024];
	wcstombs_s(&iConverted, sIPAddress, sizeof(sIPAddress), wsIPAddress, wcslen(wsIPAddress) * sizeof(WCHAR));
#endif

	SOCKADDR_IN RemoteServerSocketAddress;

	SOCKET sckLocalSocket = 0;

	if (this->p_iCurrentClients == this->p_iMaxConnections)
	{
		this->Unlock();
		return this->p_ThrowError(26);
	}

	RemoteServerSocketAddress.sin_family = AF_INET;
	RemoteServerSocketAddress.sin_port = htons(iRemotePort);
	RemoteServerSocketAddress.sin_addr.s_addr = inet_addr(sIPAddress);

	sckLocalSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (connect(sckLocalSocket,
		(struct sockaddr *) &RemoteServerSocketAddress,
		sizeof(RemoteServerSocketAddress)) == SOCKET_ERROR)
	{
		shutdown(sckLocalSocket, SD_BOTH);
		closesocket(sckLocalSocket);
		this->Unlock();
		return this->p_ThrowError(33);
	}

	int iSlot = 0;
	CSocketClient *pClient = this->p_GetFreeSocketSlot(&iSlot);
	*pOutClient = pClient;

	if (!pClient)
	{
		shutdown(sckLocalSocket, SD_BOTH);
		closesocket(sckLocalSocket);
		this->Unlock();
		return this->p_ThrowError(27);
	}

	pClient->iSlot = iSlot;
	pClient->bWasConnect = true;
	pClient->bConnected = true;
	pClient->bDisconnect = false;
	pClient->Socket = sckLocalSocket;
	pClient->hClientThread = NULL;

	pClient->iPeerID = this->p_iNextPeerID++;
	pClient->iPeerPort = iRemotePort;
#ifdef UNICODE
	wcscpy_s(pClient->sPeerAddress, sizeof(pClient->sPeerAddress) / sizeof(WCHAR), wsIPAddress);
#else
	strcpy_s(pClient->sPeerAddress, sizeof(pClient->sPeerAddress), sIPAddress);
#endif

	if (this->pOnConnect)
	{
		if (!this->pOnConnect(this, pClient))
		{
			shutdown(sckLocalSocket, SD_BOTH);
			closesocket(sckLocalSocket);

			this->p_ResetClientSlot(pClient);

			this->Unlock();
			return this->p_ThrowError(31, pClient);
		}
	}

	if (!this->p_InitializeClientVariables(pClient))
	{
		shutdown(sckLocalSocket, SD_BOTH);
		closesocket(sckLocalSocket);

		this->p_ResetClientSlot(pClient);

		this->Unlock();
		return this->p_ThrowError(28);
	}

	this->p_iCurrentClients++;

	if (bCreateThread)
	{
		bool bResult = this->p_StartClientThread(pClient);
		this->Unlock();
		return bResult;
	}
	else if (bUseCurrentThread)
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
		CTSI.bIsNewThread = false;

		this->Unlock();
		p_CilentThreadProc(&CTSI);
		return true;
	}

	this->Unlock();
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Establishes a new connection to a remote peer by IP or Hostname then returns.
*/
#ifdef UNICODE
bool CSocketServer::Connect(const WCHAR *sHostOrIP, int iRemotePort, CSocketClient **pOutClient)
#else
bool CSocketServer::Connect(const char *sHostOrIP, int iRemotePort, CSocketClient **pOutClient)
#endif
{
	if (this->IsIPAddress(sHostOrIP))
	{
		return this->ConnectByIP(sHostOrIP, iRemotePort, pOutClient);
	}
	else {
		return this->ConnectByHost(sHostOrIP, iRemotePort, pOutClient);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Establishes a new connection to a remote peer by Hostname then returns.
*/
#ifdef UNICODE
bool CSocketServer::ConnectByHost(const WCHAR *sHostName, int iRemotePort, CSocketClient **pOutClient)
{
	WCHAR sIPAddress[16];
	if (this->ResolveHost(sHostName, sIPAddress, sizeof(sIPAddress)))
	{
		return this->ConnectByIP(sIPAddress, iRemotePort, pOutClient);
	}
	return false;
}
#else
bool CSocketServer::ConnectByHost(const char *sHostName, int iRemotePort, CSocketClient **pOutClient)
{
	char sIPAddress[16];
	if (this->ResolveHost(sHostName, sIPAddress, sizeof(sIPAddress)))
	{
		return this->ConnectByIP(sIPAddress, iRemotePort, pOutClient);
	}
	return false;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Establishes a new connection to a remote peer by IP then returns.
*/
#ifdef UNICODE
bool CSocketServer::ConnectByIP(const WCHAR *sIPAddress, int iRemotePort, CSocketClient **pOutClient)
#else
bool CSocketServer::ConnectByIP(const char *sIPAddress, int iRemotePort, CSocketClient **pOutClient)
#endif
{
	return this->p_Connect(sIPAddress, iRemotePort, pOutClient, false, false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Establishes a new connection to a remote peer by IP or Hostname,
		creates a new thread to handle the client's transactions then returns.
*/
#ifdef UNICODE
bool CSocketServer::ConnectAndCreateThread(const WCHAR *sHostOrIP, int iRemotePort, CSocketClient **pOutClient)
#else
bool CSocketServer::ConnectAndCreateThread(const char *sHostOrIP, int iRemotePort, CSocketClient **pOutClient)
#endif
{
	if (this->IsIPAddress(sHostOrIP))
	{
		return this->ConnectByIPAndCreateThread(sHostOrIP, iRemotePort, pOutClient);
	}
	else {
		return this->ConnectByHostAndCreateThread(sHostOrIP, iRemotePort, pOutClient);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Establishes a new connection to a remote peer by Hostname,
		creates a new thread to handle the client's transactions then returns.
*/
#ifdef UNICODE
bool CSocketServer::ConnectByHostAndCreateThread(const WCHAR *sHostName, int iRemotePort, CSocketClient **pOutClient)
{
	WCHAR sIPAddress[16];
	if (this->ResolveHost(sHostName, sIPAddress, sizeof(sIPAddress)))
	{
		return this->ConnectByIPAndCreateThread(sIPAddress, iRemotePort, pOutClient);
	}
	return false;
}
#else
bool CSocketServer::ConnectByHostAndCreateThread(const char *sHostName, int iRemotePort, CSocketClient **pOutClient)
{
	char sIPAddress[16];
	if (this->ResolveHost(sHostName, sIPAddress, sizeof(sIPAddress)))
	{
		return this->ConnectByIPAndCreateThread(sIPAddress, iRemotePort, pOutClient);
	}
	return false;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Establishes a new connection to a remote peer by IP,
		creates a new thread to handle the client's transactions then returns.
*/
#ifdef UNICODE
bool CSocketServer::ConnectByIPAndCreateThread(const WCHAR *sIPAddress, int iRemotePort, CSocketClient **pOutClient)
#else
bool CSocketServer::ConnectByIPAndCreateThread(const char *sIPAddress, int iRemotePort, CSocketClient **pOutClient)
#endif
{
	return this->p_Connect(sIPAddress, iRemotePort, pOutClient, true, false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Establishes a new connection to a remote peer by IP or Hostname,
		uses the current thread to handle the client's transactions
		then returns after the client disconnects.
*/
#ifdef UNICODE
bool CSocketServer::ConnectUseCurrentThread(const WCHAR *sHostOrIP, int iRemotePort)
#else
bool CSocketServer::ConnectUseCurrentThread(const char *sHostOrIP, int iRemotePort)
#endif
{
	if (this->IsIPAddress(sHostOrIP))
	{
		return this->ConnectByIPUseCurrentThread(sHostOrIP, iRemotePort);
	}
	else {
		return this->ConnectByHostUseCurrentThread(sHostOrIP, iRemotePort);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Establishes a new connection to a remote peer by Hostname,
		uses the current thread to handle the client's transactions
		then returns after the client disconnects.
*/
#ifdef UNICODE
bool CSocketServer::ConnectByHostUseCurrentThread(const WCHAR *sHostName, int iRemotePort)
{
	WCHAR sIPAddress[16];
	if (this->ResolveHost(sHostName, sIPAddress, sizeof(sIPAddress)))
	{
		return this->ConnectByIPUseCurrentThread(sIPAddress, iRemotePort);
	}
	return false;
}
#else
bool CSocketServer::ConnectByHostUseCurrentThread(const char *sHostName, int iRemotePort)
{
	char sIPAddress[16];
	if (this->ResolveHost(sHostName, sIPAddress, sizeof(sIPAddress)))
	{
		return this->ConnectByIPUseCurrentThread(sIPAddress, iRemotePort);
	}
	return false;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Establishes a new connection to a remote peer by IP,
		uses the current thread to handle the client's transactions
		then returns after the client disconnects.
*/
#ifdef UNICODE
bool CSocketServer::ConnectByIPUseCurrentThread(const WCHAR *sIPAddress, int iRemotePort)
#else
bool CSocketServer::ConnectByIPUseCurrentThread(const char *sIPAddress, int iRemotePort)
#endif
{
	CSocketClient *pClient = NULL;
	return this->p_Connect(sIPAddress, iRemotePort, &pClient, false, true);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
