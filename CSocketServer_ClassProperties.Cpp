///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_ClassProperties_Cpp
#define _CSocketServer_ClassProperties_Cpp
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
	This function returns a pointer to one of the client slots by its Peer ID.
*/
CSocketClient *CSocketServer::Peer(int iPeerID)
{
	for (int iSlot = 0; iSlot < this->p_iMaxConnections; iSlot++)
	{
		CSocketClient *pClient = this->Client(iSlot);

		if (pClient && pClient->iPeerID == iPeerID)
		{
			return &this->p_Clients[iSlot];
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function returns a pointer to one of the client slots by is Client ID
*/
CSocketClient *CSocketServer::Client(int iSlot)
{
	if (iSlot < 0 || iSlot >= this->p_iMaxConnections)
	{
		return NULL;
	}
	return &this->p_Clients[iSlot];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function can be used to poll for connected clients (blocking).

	Call and it will pass out a pointer to a qualified
		(connected and not pending disconnection) connected client.
*/
CSocketClient *CSocketServer::GetNextConnectedClient(void)
{
	static int iLastClient = 0;
	int iStartingPoint = iLastClient;

	if (iLastClient < 0 || iLastClient >= this->p_iMaxConnections)
	{
		iLastClient = 0;
	}

	while (this->IsActive())
	{
		//Loop from where we left off last time, to the MAX count of clients.
		for (;iLastClient < this->p_iMaxConnections; iLastClient++)
		{
			if (this->p_Clients[iLastClient].IsConnected())
			{
				return &this->p_Clients[iLastClient++];
			}
		}
		iLastClient = 0;

		//Loop from slot 0 to where the top loop started, this makes
		//	sure we will never Sleep() while clients are connected.
		for (;iLastClient < iStartingPoint; iLastClient++)
		{
			if (this->p_Clients[iLastClient].IsConnected())
			{
				return &this->p_Clients[iLastClient++];
			}
		}
		iStartingPoint = 0;

		Sleep(1);
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	This function can be used to poll for connected clients (blocking).

	Pass in a pointer to an integer, and it will pass out the client
		slot number of a qualified (connected and not pending disconnection) connected client.
*/
bool CSocketServer::GetNextConnectedClient(int *iSlot)
{
	static int iLastClient = 0;
	int iStartingPoint = iLastClient;

	if (iLastClient < 0 || iLastClient >= this->p_iMaxConnections)
	{
		iLastClient = 0;
	}

	while (this->IsActive())
	{
		//Loop from where we left off last time, to the MAX count of clients.
		for (;iLastClient < this->p_iMaxConnections; iLastClient++)
		{
			if (this->p_Clients[iLastClient].IsConnected())
			{
				*iSlot = iLastClient++;
				return true;
			}
		}
		iLastClient = 0;

		//Loop from slot 0 to where the top loop started, this makes
		//	sure we will never Sleep() while clients are connected.
		for (;iLastClient < iStartingPoint; iLastClient++)
		{
			if (this->p_Clients[iLastClient].IsConnected())
			{
				*iSlot = iLastClient++;
				return true;
			}
		}
		iStartingPoint = 0;

		Sleep(1);
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function can be used to poll for connected clients (non-blocking).

	Call and it will pass out a pointer to a qualified
		(connected and not pending disconnection) connected client.
*/
CSocketClient *CSocketServer::GetConnectedClient(void)
{
	static int iLastClient = 0;
	int iStartingPoint = iLastClient;

	if (iLastClient < 0 || iLastClient >= this->p_iMaxConnections)
	{
		iLastClient = 0;
	}

	//Loop from where we left off last time, to the MAX count of clients.
	for (;iLastClient < this->p_iMaxConnections; iLastClient++)
	{
		if (this->p_Clients[iLastClient].IsConnected())
		{
			return &this->p_Clients[iLastClient++];
		}
	}
	iLastClient = 0;

	//Loop from slot 0 to where the top loop started, this makes
	//	sure we will never Sleep() while clients are connected.
	for (;iLastClient < iStartingPoint; iLastClient++)
	{
		if (this->p_Clients[iLastClient].IsConnected())
		{
			return &this->p_Clients[iLastClient++];
		}
	}
	iStartingPoint = 0;

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	This function can be used to poll for connected clients (non-blocking).

	Pass in a pointer to an integer, and it will pass out the client
		slot number of a qualified (connected and not pending disconnection) connected client.
*/
bool CSocketServer::GetConnectedClient(int *iSlot)
{
	static int iLastClient = 0;
	int iStartingPoint = iLastClient;

	if (iLastClient < 0 || iLastClient >= this->p_iMaxConnections)
	{
		iLastClient = 0;
	}

	//Loop from where we left off last time, to the MAX count of clients.
	for (;iLastClient < this->p_iMaxConnections; iLastClient++)
	{
		if (this->p_Clients[iLastClient].IsConnected())
		{
			*iSlot = iLastClient++;
			return true;
		}
	}
	iLastClient = 0;

	//Loop from slot 0 to where the top loop started, this makes
	//	sure we will never Sleep() while clients are connected.
	for (;iLastClient < iStartingPoint; iLastClient++)
	{
		if (this->p_Clients[iLastClient].IsConnected())
		{
			*iSlot = iLastClient++;
			return true;
		}
	}
	iStartingPoint = 0;

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the socket server is in "packet mode", else returns false.

	Enabling / Disabling "Packet Mode" only sets the default for new connections,
		it does not change the "Packet Mode" for existing connectios.

*/
bool CSocketServer::PacketMode(void)
{
	return this->p_bPacketMode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Enables / Disables "packet mode" for the socket server.

	Enabling / Disabling "Packet Mode" only sets the default for new connections,
		it does not change the "Packet Mode" for existing connectios.

*/
void CSocketServer::PacketMode(bool bUsePacketing)
{
	this->p_bPacketMode = bUsePacketing;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function sets the buffer size.
*/
void CSocketServer::InitialBufferSize(int iSize)
{
	if (this->p_iInitialBufferSize > 0)
	{
		this->p_iInitialBufferSize = iSize;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function returns the buffer size.
*/
int CSocketServer::InitialBufferSize(void)
{
	return this->p_iInitialBufferSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function sets the max buffer size. (0=Infinite)

*/
void CSocketServer::MaxBufferSize(int iSize)
{
	this->p_iMaxBufferSize = iSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function returns the max buffer size. (0=Infinite)
*/
int CSocketServer::MaxBufferSize(void)
{
	return this->p_iMaxBufferSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function returns listen port for the server.
*/
int CSocketServer::ListenPort(void)
{
	return this->p_iListenPort;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function sets the listen port then returns the previous port for the server.
*/
int CSocketServer::ListenPort(int iNewPort)
{
	int iOldPort = this->p_iListenPort;
	this->p_iListenPort = iNewPort;
	return iOldPort;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function returns true if the client / server is running.
*/
bool CSocketServer::IsActive(void)
{
	return this->p_bActive;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function returns true if the class has been initilized.
*/
bool CSocketServer::IsInitialized(void)
{
	return this->p_bIsInitialized;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to set enable the Accept / Connect functionality.
*/
void CSocketServer::AllowNewConnections(bool bAllow)
{
	this->p_bAllowNewConnections = bAllow;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function returns true if the class was initialized as a server. False is client only.
*/
bool CSocketServer::IsListeningServer(void)
{
	return this->p_bIsListeningServer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the pointer to used defined per class instance data.
*/
VOID *CSocketServer::UserData(void)
{
	return this->pUserData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the pointer to used defined per class instance data.
*/
void CSocketServer::UserData(VOID *pData)
{
	this->pUserData = pData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to set p_iPacketHeader to a new value.
*/
void CSocketServer::PacketHeader(int iPacketHeader)
{
	this->p_iPacketHeader = iPacketHeader;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to get the value of p_iPacketHeader.
*/
int CSocketServer::PacketHeader(void)
{
	return this->p_iPacketHeader;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to set p_iNextPeerID to a new value.
*/
void CSocketServer::NextPeerID(int iPeerID)
{
	this->p_iNextPeerID = iPeerID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to get the value of p_iNextPeerID.
*/
int CSocketServer::NextPeerID(void)
{
	return this->p_iNextPeerID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to get the value of p_iMaxConnections.
*/
int CSocketServer::MaxConnections(void)
{
	return this->p_iMaxConnections;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to get the count of currently connected clients.
*/
int CSocketServer::CurrentConnections(void)
{
	return this->p_iCurrentClients;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Optimizing for throughput will allow the socket server to use additional CPU time to aid in high bit-rates.
*/
bool CSocketServer::OptimizeForThroughput(bool bValue)
{
	bool bPreviousValue = this->p_bOptimizeForThroughput;
	this->p_bOptimizeForThroughput = bValue;
	return bPreviousValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Optimizing for throughput will allow the socket server to use additional CPU time to aid in high bit-rates.
*/

bool CSocketServer::OptimizeForThroughput(void)
{
	return this->p_bOptimizeForThroughput;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSocketServer::ForceSocketReuse(bool bValue)
{
	bool bPreviousValue = this->p_bForceSocketReuse;
	this->p_bForceSocketReuse = bValue;
	return bPreviousValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSocketServer::ForceSocketReuse(void)
{
	return this->p_bForceSocketReuse;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the current size of the internal buffer.
*/
int CSocketServer::GetBufferSize(void)
{
	return this->p_Buffer.iAlloc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the current Max Inactivity Time brfore a peer is disconnected.
*/
double CSocketServer::MaxInactivityTime(void)
{
	return this->p_dMaxInactivityTime;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Sets the Max Inactivity Time brfore a peer is disconnected.
*/
void CSocketServer::MaxInactivityTime(double dSeconds)
{
	this->p_dMaxInactivityTime = dSeconds;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
