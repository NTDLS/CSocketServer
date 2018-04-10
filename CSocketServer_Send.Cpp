///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_Send_Cpp
#define _CSocketServer_Send_Cpp
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
	Sends data to a remote peer.

	This function should not be called directly!
*/
bool CSocketServer::p_Send(CSocketClient *pClient)
{
	if (pClient->SendChunk.iLength > 0)
	{
		if (this->pOnSend)
		{
			if (!this->pOnSend(this, pClient, &pClient->SendChunk))
			{
				return false; //All is well.
			}
		}

		if (pClient->IsConnected())
		{
			this->p_Packetize(pClient);

			int iResult = send(pClient->Socket, (char *)this->p_Buffer.pBuffer, this->p_Buffer.iLength, 0);

			if (iResult == SOCKET_ERROR)
			{
				int iLastError = WSAGetLastError();
				if (iLastError == WSAEWOULDBLOCK || iLastError == WSAEINPROGRESS || iLastError == WSAEALREADY)
				{
					return false;
				}
				else if (iLastError == WSAENOBUFS)
				{
					this->p_ThrowError(11, pClient);
					pClient->bDisconnect = true;
				}
				else {
					pClient->bDisconnect = true;
				}
			}
			else if (iResult == 0)
			{
				pClient->bDisconnect = true;
			}
			else if (iResult != this->p_Buffer.iLength)
			{
				this->p_ThrowError(11, pClient);
				pClient->bDisconnect = true;
			}
			else {
				pClient->SendChunk.iLength = 0; //All is well.
				time(&pClient->ttLastActivity);
				SetEvent(pClient->hDataSent);

				return true;
			}
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
