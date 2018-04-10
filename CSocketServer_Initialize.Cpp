///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_Initialize_Cpp
#define _CSocketServer_Initialize_Cpp
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
	This function is used to Initialize all of the class variables for a connecting client.
*/
bool CSocketServer::Initialize(ClientHandlerThread lpClientThread)
{
	bool bResult = this->Initialize(0, 1, (OnError)NULL);
	if (bResult)
	{
		this->SetClientThread(lpClientThread);
	}
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to Initialize all of the class variables for a connecting client.
*/
bool CSocketServer::Initialize(ClientHandlerThread lpClientThread, OnError lpOnError)
{
	bool bResult = this->Initialize(0, 1, lpOnError);
	if (bResult)
	{
		this->SetClientThread(lpClientThread);
	}
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to Initialize all of the class variables for a connecting client.
*/
bool CSocketServer::Initialize(OnError lpOnError)
{
	return this->Initialize(0, 1, lpOnError);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to Initialize all of the class variables for a listening server.
*/
bool CSocketServer::Initialize(int iListenPort, int iMaxConnections,
	ClientHandlerThread lpClientThread)
{
	bool bResult = this->Initialize(iListenPort, iMaxConnections, (OnError)NULL);
	if (bResult)
	{
		this->SetClientThread(lpClientThread);
	}
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to Initialize all of the class variables for a listening server.
*/
bool CSocketServer::Initialize(int iListenPort, int iMaxConnections,
	ClientHandlerThread lpClientThread, OnError lpOnError)
{
	bool bResult = this->Initialize(iListenPort, iMaxConnections, (OnError)lpOnError);
	this->SetClientThread(lpClientThread);
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to Initialize all of the class variables for a listening server.
*/
bool CSocketServer::Initialize(int iListenPort, int iMaxConnections, OnError lpOnError)
{
	srand(GetTickCount());

	this->p_bActive = false;
	this->p_bIsInitialized = true;

	memset(&this->p_ListenAddrs, 0, sizeof(this->p_ListenAddrs));
	memset(&this->p_ListenSockets, 0, sizeof(this->p_ListenSockets));

	//Defaut event handlers.
	this->pOnAccept = NULL;
	this->pOnAcceptConnect = NULL;
	this->pOnAfterDePacketize = NULL;
	this->pOnAfterPacketize = NULL;
	this->pOnBeforeDePacketize = NULL;
	this->pOnBeforePacketize = NULL;
	this->pOnBeginClientThread = NULL;
	this->pOnConnect = NULL;
	this->pOnDisconnect = NULL;
	this->pOnEndClientThread = NULL;
	this->pOnError = lpOnError;
	this->pOnConsume = NULL;
	this->pOnRecv = NULL;
	this->pOnSend = NULL;
	this->pOnStart = NULL;
	this->pOnStop = NULL;
	this->pOnClientPump = NULL;
	this->pOnPump = NULL;

	if (!this->p_StartWindowsSocket())
	{
		return false;
	}

	if (iMaxConnections > this->p_iSocketLimit)
	{
		this->p_StopWindowsSocket();
		return this->p_ThrowError(32);
	}

	if (iMaxConnections > FD_SETSIZE)
	{
		this->p_StopWindowsSocket();
		return this->p_ThrowError(29);
	}

	this->p_pClientHandlerThread = NULL;
	this->p_iListenPort = iListenPort;
	this->p_iMaxConnections = iMaxConnections;
	this->p_iCurrentClients = 0;
	this->p_iNextPeerID = 0;
	this->p_iInitialBufferSize = SSDFLTINITBUFSZ;
	this->p_iMaxBufferSize = SSDFLTMAXBUFSZ;
	this->p_iPacketHeader = SSPACKETHEADER;
	this->p_dMaxInactivityTime = 0;
	this->p_bIsListeningServer = false;
	this->p_bAllowNewConnections = false;
	this->p_bPacketMode = true;
	this->p_bOptimizeForThroughput = true;
	this->p_bForceSocketReuse = false;

	//Initialize the Critical Section objects.
	InitializeCriticalSection(&this->p_TcpPumpLock);
	InitializeCriticalSection(&this->p_HostResolverLock);
#ifndef _USE_GLOBAL_MEMPOOL
	InitializeCriticalSection(&this->p_MemoryLock);
#endif

	//Allocate memory for all of our class variables.
	if (!this->Alloc(&this->p_Buffer, this->p_iInitialBufferSize))
	{
		this->p_StopWindowsSocket();
		return false;
	}

	if ((this->p_Clients = (CSocketClient *) this->Alloc(sizeof(CSocketClient), iMaxConnections)) == NULL)
	{
		this->Free(&this->p_Buffer);
		this->p_StopWindowsSocket();
		return false;
	}

	//Set all of our Client variables to their default values.
	for (int iSlot = 0; iSlot < iMaxConnections; iSlot++)
	{
		this->p_ResetClientSlot(&this->p_Clients[iSlot]);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to Destroy all of the class variables.
		Call this function when you are done with the class.
*/
bool CSocketServer::Destroy(void)
{
	if (this->p_bActive)
	{
		if (this->p_iCurrentClients > 0) //Force disconnection.
		{
			for (int iSlot = 0; iSlot < this->p_iMaxConnections; iSlot++)
			{
				this->Lock();
				if (this->p_Clients[iSlot].IsConnected())
				{
					this->p_DisconnectClient(&this->p_Clients[iSlot]);
				}
				this->Unlock();
			}
		}

		this->Stop();
	}

	this->p_bActive = false;
	this->p_bIsInitialized = false;

	if (&this->p_Buffer) this->Free(&this->p_Buffer);
	if (this->p_Clients) this->Free(this->p_Clients);

	if (this->p_ListenSockets.Count > 0)
	{
		this->Free(this->p_ListenSockets.Socket);
	}

	this->ClearListenAddresses();

	//Delete our Critical Section objects.
	DeleteCriticalSection(&this->p_TcpPumpLock);
	DeleteCriticalSection(&this->p_HostResolverLock);

#ifndef _USE_GLOBAL_MEMPOOL
	DeleteCriticalSection(&this->p_MemoryLock);
#endif

	return this->p_StopWindowsSocket();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
