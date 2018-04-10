///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_Accept_Cpp
#define _CSocketServer_Accept_Cpp
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function accepts a new connection from a remote peer.
	This function should not be called directly!
*/
bool CSocketServer::p_Accept(SOCKET listenSocket)
{
	if (!this->p_bAllowNewConnections)
	{
		return false;
	}

	int iLength = 0;
	SOCKET sckLocalSocket = 0;

	SOCKADDR_IN RemoteClientSocketAddress;

	iLength = sizeof(SOCKADDR);
	if ((sckLocalSocket = accept(listenSocket, (SOCKADDR*)&RemoteClientSocketAddress, &iLength)) == SOCKET_ERROR)
	{
		int iLastError = WSAGetLastError();
		if (iLastError == WSAEWOULDBLOCK || iLastError == WSAEINPROGRESS || iLastError == WSAEALREADY)
		{
			return false;
		}

		return this->p_ThrowError(3);
	}
	else if (sckLocalSocket == 0)
	{
		return this->p_ThrowError(4);
	}

	if (this->p_iCurrentClients >= this->p_iMaxConnections)
	{
		shutdown(sckLocalSocket, SD_BOTH);
		closesocket(sckLocalSocket);
		return this->p_ThrowError(5);
	}

	int iSlot = 0;
	CSocketClient *pClient = this->p_GetFreeSocketSlot(&iSlot);
	if (!pClient)
	{
		shutdown(sckLocalSocket, SD_BOTH);
		closesocket(sckLocalSocket);
		return this->p_ThrowError(6);
	}

	pClient->iSlot = iSlot;
	pClient->bWasConnect = false;
	pClient->bConnected = true;
	pClient->bDisconnect = false;
	pClient->Socket = sckLocalSocket;

	pClient->iPeerID = this->p_iNextPeerID++;
	pClient->iPeerPort = ntohs(RemoteClientSocketAddress.sin_port);

#ifdef UNICODE
	size_t iConverted = 0;
	char *sAddress = inet_ntoa(RemoteClientSocketAddress.sin_addr);
	mbstowcs_s(&iConverted, pClient->sPeerAddress,
		sizeof(pClient->sPeerAddress) / sizeof(WCHAR),
		sAddress, strlen(sAddress));
#else
	strcpy_s(pClient->sPeerAddress,
		sizeof(pClient->sPeerAddress),
		inet_ntoa(RemoteClientSocketAddress.sin_addr));
#endif

	if (this->pOnAccept)
	{
		if (!this->pOnAccept(this, pClient))
		{
			//WSASendDisconnect(sckLocalSocket, NULL);
			shutdown(sckLocalSocket, SD_BOTH);
			closesocket(sckLocalSocket);

			this->p_ResetClientSlot(pClient);

			return this->p_ThrowError(8, pClient);
		}
	}

	if (!this->p_InitializeClientVariables(pClient))
	{
		shutdown(sckLocalSocket, SD_BOTH);
		closesocket(sckLocalSocket);

		this->p_ResetClientSlot(pClient);

		return this->p_ThrowError(7, pClient);
	}

	this->p_iCurrentClients++;

	return this->p_StartClientThread(pClient);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
