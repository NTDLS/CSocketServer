///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_Receive_Cpp
#define _CSocketServer_Receive_Cpp
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
	Receives data from a remote peer.

	This function should not be called directly!
*/
bool CSocketServer::p_Receive(CSocketClient *pClient)
{
	if (pClient->RecvChunk.iLength > this->p_iMaxBufferSize && this->p_iMaxBufferSize > 0)
	{
		if (pClient->bPacketMode)
		{
			if (this->p_AnalysePacket(pClient) == SSPACKET_GOOD_PACKET)
			{
				//We are already over the buffer quota and the client has a
				//	full packet in their buffer. No reason to receive any
				//	more data at this point because we are receiving data
				//	faster then it is being process and all we'll do it 
				//	waste RAM.
				return false;
			}
			else {
				//We're in Packet Mode. If we havent received a full packet
				//	yet, then we have to allow the buffer to go "over-size".
			}
		}
		else {
			//We're over the buffer quota, wait for the client
			//	thread to catch-up before receiving more data.
			return false;
		}
	}

	int iMaxRecvLength = this->p_Buffer.iAlloc;
	if (!pClient->bPacketMode && iMaxRecvLength > this->p_iMaxBufferSize)
	{
		//Try to keep the client buffer as small as possible.
		iMaxRecvLength = this->p_iMaxBufferSize;
	}

	this->p_Buffer.iLength = recv(pClient->Socket,
		(char *) this->p_Buffer.pBuffer, iMaxRecvLength, 0);

	if (this->p_Buffer.iLength == SOCKET_ERROR)
	{
		pClient->bDisconnect = true;
	}
	else if (this->p_Buffer.iLength == 0) // Gracefull disconnect
	{
		pClient->bDisconnect = true;
	}
	else {
		if (this->pOnRecv)
		{
			if (!this->pOnRecv(this, pClient, &this->p_Buffer))
			{
				return false;
			}
		}

		int iMemoryRequired = pClient->RecvChunk.iLength + this->p_Buffer.iLength;
		if (iMemoryRequired >= pClient->RecvChunk.iAlloc)
		{
			if (!this->ReAlloc(&pClient->RecvChunk, iMemoryRequired + 1))
			{
				return false;
			}
		}

		memcpy_s(((BYTE *)pClient->RecvChunk.pBuffer) + pClient->RecvChunk.iLength,
			pClient->RecvChunk.iAlloc, this->p_Buffer.pBuffer, this->p_Buffer.iLength);

		pClient->RecvChunk.iLength += this->p_Buffer.iLength;

		if (pClient->bPacketMode)
		{
			if (this->p_AnalysePacket(pClient) < 0)
			{
				pClient->bDisconnect = true;
				return this->p_ThrowError(1, pClient);
			}
		}

		time(&pClient->ttLastActivity);
		SetEvent(pClient->hDataReceived);

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
