///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_Packetize_Cpp
#define _CSocketServer_Packetize_Cpp
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Creates a packet from a clients pending send data. This function should not be called directly!
*/
int CSocketServer::p_Packetize(CSocketClient *pClient)
{
	if (!pClient->bPacketMode)
	{
		if (this->p_Buffer.iAlloc <= pClient->SendChunk.iLength)
		{
			if (!this->ReAlloc(&this->p_Buffer, pClient->SendChunk.iLength + 1))
			{
				return SSPACKET_MEMORY_ERROR;
			}
		}

		//Copy the packet data to the output block.
		memcpy_s(this->p_Buffer.pBuffer,
			this->p_Buffer.iAlloc,
			pClient->SendChunk.pBuffer,
			pClient->SendChunk.iLength);

		this->p_Buffer.iLength = pClient->SendChunk.iLength;
		((BYTE *)this->p_Buffer.pBuffer)[this->p_Buffer.iLength] = '\0';
		return this->p_Buffer.iLength;
	}

	if (this->pOnBeforePacketize)
	{
		if (!this->pOnBeforePacketize(this, pClient, &pClient->SendChunk))
		{
			return SSPACKET_USER_FILTERED;
		}
	}

	if (this->p_Buffer.iAlloc <= pClient->SendChunk.iLength + SSPACKETHEADERSZ)
	{
		if (!this->ReAlloc(&this->p_Buffer, pClient->SendChunk.iLength + SSPACKETHEADERSZ + 1))
		{
			return SSPACKET_MEMORY_ERROR;
		}
	}

	//Append the packet header.
	memcpy_s(this->p_Buffer.pBuffer,
		this->p_Buffer.iAlloc,
		&this->p_iPacketHeader,
		sizeof(this->p_iPacketHeader));

	//Append the full packet size.
	memcpy_s(((BYTE *)this->p_Buffer.pBuffer) + sizeof(this->p_iPacketHeader),
		this->p_Buffer.iAlloc,
		&pClient->SendChunk.iLength,
		sizeof(pClient->SendChunk.iLength));

	//Append the packet data.
	memcpy_s(((BYTE *)this->p_Buffer.pBuffer) + SSPACKETHEADERSZ,
		this->p_Buffer.iAlloc,
		pClient->SendChunk.pBuffer,
		pClient->SendChunk.iLength);

	this->p_Buffer.iLength = pClient->SendChunk.iLength + SSPACKETHEADERSZ;
	((BYTE *)this->p_Buffer.pBuffer)[this->p_Buffer.iLength] = '\0';

	if (this->pOnAfterPacketize)
	{
		if (!this->pOnAfterPacketize(this, pClient, &this->p_Buffer))
		{
			return SSPACKET_USER_FILTERED;
		}
	}

	return this->p_Buffer.iLength; // The total length of the packet.
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Analyses received client data to determine if they are full, partial or invalid packets.
	This function should not be called directly!
*/
int CSocketServer::p_AnalysePacket(CSocketClient *pClient)
{
	int iPacketHeader = 0;
	int iReportedLength = 0;

	if (!pClient->bPacketMode)
	{
		return SSPACKET_GOOD_PACKET;
	}

	//Read the packet header.
	memcpy_s(&iPacketHeader, sizeof(iPacketHeader),
		pClient->RecvChunk.pBuffer,
		sizeof(iPacketHeader));

	if (iPacketHeader != this->p_iPacketHeader)
	{
		return SSPACKET_INVALID_HEADER;
	}

	//Read the size of the full packet (we may have received less data, which is OK).
	memcpy_s(&iReportedLength, sizeof(iReportedLength),
		((BYTE *)pClient->RecvChunk.pBuffer) + sizeof(iPacketHeader), sizeof(iReportedLength));

	//If the data chunk is less than or equal to zero bytes, the packet must be rejected.
	if (iReportedLength <= 0)
	{
		return SSPACKET_INVALID_PACKET;
	}

	if ((iReportedLength + SSPACKETHEADERSZ) > pClient->RecvChunk.iLength)
	{
		//The receive buffer currently contains a partial packet.
		//More data will have to be received before this data can be processed.
		return SSPACKET_PARTIAL_DATA;
	}

	return SSPACKET_GOOD_PACKET;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function retreves the data segment from a clients packet.
	This function should not be called directly!
*/
int CSocketServer::p_DePacketize(CSocketClient *pClient, BASICHUNK *pChunk)
{
	if (!pClient->bPacketMode)
	{
		if (pChunk->iAlloc <= pClient->RecvChunk.iLength)
		{
			if (!this->ReAlloc(pChunk, pClient->RecvChunk.iLength + 1))
			{
				return SSPACKET_MEMORY_ERROR;
			}
		}

		//Copy the packet data to the output block.
		memcpy_s(pChunk->pBuffer, pChunk->iAlloc,
			pClient->RecvChunk.pBuffer,
			pClient->RecvChunk.iLength);

		pChunk->iLength = pClient->RecvChunk.iLength;
		((BYTE *)pChunk->pBuffer)[pChunk->iLength] = '\0';

		pClient->RecvChunk.iLength = 0;

		return SSPACKET_GOOD_PACKET;
	}

	int iPacketHeader = 0;
	int iReportedLength = 0;

	if (this->pOnBeforeDePacketize)
	{
		if (!this->pOnBeforeDePacketize(this, pClient, &pClient->RecvChunk))
		{
			return SSPACKET_USER_FILTERED;
		}
	}

	//Read the packet header.
	memcpy_s(&iPacketHeader, sizeof(iPacketHeader),
		pClient->RecvChunk.pBuffer, sizeof(iPacketHeader));

	if (iPacketHeader != this->p_iPacketHeader)
	{
		return SSPACKET_INVALID_HEADER;
	}

	//Read the size of the full packet (we may have received less data, which is OK).
	memcpy_s(&iReportedLength, sizeof(iReportedLength),
		((BYTE *)pClient->RecvChunk.pBuffer) + sizeof(iPacketHeader), sizeof(iReportedLength));

	//If the data chunk is less than or equal to zero bytes, the packet must be rejected.
	if (iReportedLength <= 0)
	{
		return SSPACKET_INVALID_PACKET;
	}

	if ((iReportedLength + SSPACKETHEADERSZ) > pClient->RecvChunk.iLength)
	{
		//The receive buffer currently contains a partial packet.
		//More data will have to be received before this data can be processed.
		return SSPACKET_PARTIAL_DATA;
	}

	if (pChunk->iAlloc <= iReportedLength)
	{
		if (!this->ReAlloc(pChunk, iReportedLength + 1))
		{
			return SSPACKET_MEMORY_ERROR;
		}
	}

	//Copy the packet data to the output block.
	memcpy_s(pChunk->pBuffer,
		pChunk->iAlloc,
		((BYTE *)pClient->RecvChunk.pBuffer) + SSPACKETHEADERSZ,
		iReportedLength);

	pChunk->iLength = iReportedLength;
	((BYTE *)pChunk->pBuffer)[pChunk->iLength] = '\0';

	if ((pClient->RecvChunk.iLength -= (pChunk->iLength + SSPACKETHEADERSZ)) > 0)
	{
		//Reposition the remaining class buffer data to position zero.
		memcpy_s(pClient->RecvChunk.pBuffer,
			pClient->RecvChunk.iLength,
			((BYTE *)pClient->RecvChunk.pBuffer) + (pChunk->iLength + SSPACKETHEADERSZ),
			pClient->RecvChunk.iLength);
	}

	if (this->pOnAfterDePacketize)
	{
		if (!this->pOnAfterDePacketize(this, pClient, pChunk))
		{
			return SSPACKET_USER_FILTERED;
		}
	}

	return SSPACKET_GOOD_PACKET;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
