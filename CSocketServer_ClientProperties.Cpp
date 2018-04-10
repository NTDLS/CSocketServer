///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_ClientProperties_Cpp
#define _CSocketServer_ClientProperties_Cpp
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
	Returns true if the client is in "packet mode", else returns false.
*/
bool CSocketClient::PacketMode(void)
{
	return this->bPacketMode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Enables / Disables "packet mode" for a connected client.

	New connections will still inherit the "Packet Mode" value from the socket server.
*/
void CSocketClient::PacketMode(bool bUsePacketing)
{
	this->bPacketMode = bUsePacketing;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	The client event handle which is set when data arrives for this client.
*/
HANDLE CSocketClient::EventDataReceivedWaitHandle(void)
{
	return this->hDataReceived;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	The client event handle which is set when data departs for this client.
*/
HANDLE CSocketClient::EventDataSentWaitHandle(void)
{
	return this->hDataSent;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns a pointer to user defined (per client) data.
	This is 100% user maintained - get / set the pUserData to anything you want.
*/
VOID *CSocketClient::UserData(void)
{
	return this->pUserData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Sets the pointer to user defined (per client) data.

	This is 100% user maintained - get / set the pUserData to anything you want.
*/
void CSocketClient::UserData(VOID *pData)
{
	this->pUserData = pData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns True if the client was connected, false if the client was accepted.
*/
bool CSocketClient::WasConnect(void)
{
	return this->bWasConnect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function reserves a client slot.

	Reserved slots will not be made availabe to the free pool even
		after client disconnection, therfore all client variables are protected.
*/
void CSocketClient::Reserved(bool bReserve)
{
	this->bReserved = bReserve;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns whether a client slot is reserved or not. See Reserved() overloads for more info.
*/
bool CSocketClient::Reserved(void)
{
	return this->bReserved;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to get a clients peer Port.
*/
int CSocketClient::PeerPort(void)
{
	return this->iPeerPort;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to get a clients peer IP address.
*/
#ifdef UNICODE
WCHAR *CSocketClient::PeerAddress(void)
#else
char *CSocketClient::PeerAddress(void)
#endif
{
	return this->sPeerAddress;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to get a clients peer IP address.
*/
#ifdef UNICODE
int CSocketClient::PeerAddress(WCHAR *sOutAddress, int iMaxSz)
{
	if (wcscpy_s(sOutAddress, iMaxSz / sizeof(WCHAR), this->sPeerAddress) == 0)
	{
		return (int)wcslen(sOutAddress)
	}
	else {
		return 0;
	}
}
#else
int CSocketClient::PeerAddress(char *sOutAddress, int iMaxSz)
{
	if (strcpy_s(sOutAddress, iMaxSz, this->sPeerAddress) == 0)
	{
		return (int)strlen(sOutAddress);
	}
	else {
		return 0;
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to get a clients slot number.
*/
int CSocketClient::Slot(void)
{
	return this->iSlot;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to get a clients peer ID.

	The PeerID is used to identify a connection but should not be confused with the ClientID.

	Every time a connection is made, PeerID is incremented but is never decremented.
*/
int CSocketClient::PeerID(void)
{
	return this->iPeerID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function returns TRUE if the client has data waiting to be sent.
*/
bool CSocketClient::IsSendDataWaiting(void)
{
	// Is data waiting to be received
	return (this->SendChunk.iLength > 0 && !this->bAbortConnection);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function returns TRUE if the client has data in there receive buffer.

	NOTE: A TRUE return value does not mean that the data is a full packet,
	therfore a call to GetClientData or GetNextClientData may not return data.
*/
bool CSocketClient::IsRecvDataWaiting(void)
{
	// Is data waiting to be sent?
	return (this->RecvChunk.iLength > 0 && !this->bAbortConnection);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Sets the specified clients socket to be disconnected. Returns TRUE on success.
*/
bool CSocketClient::Disconnect(void)
{
	this->Lock();

	if (this->IsConnected())
	{
		this->bDisconnect = true;
		this->Unlock();
		return true;
	}
	else {
		this->Unlock();
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns TRUE if the specified client is set to be disconnected. This is a good way
		to determine if a client has been disconnected but is still being reported as
		connected because the slot is reserved.
*/
bool CSocketClient::IsDisconnectSet(void)
{
	return this->bDisconnect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns TRUE if the specified client is connected and not pending disconnection.
*/
bool CSocketClient::IsConnected(bool bIgnoreReserved)
{
	return ((this->bReserved && !bIgnoreReserved) || (this->bConnected && !this->bDisconnect));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns TRUE if the specified client is connected and not pending disconnection.
*/
bool CSocketClient::IsConnected(void)
{
	return (this->bReserved || (this->bConnected && !this->bDisconnect));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function will return TRUE when the client has a empty send buffer.
	Returns FALSE if the client has been disconnected or is pending a disconnect.

	Important:
		If you are working within the ClientThreadProc() and wish to send data to a connected peer other than
			the one identified by the pClient passed into the thread proc, then you must pass the 'Slot Number'
			of the client that is to receive the message. This is because both connections must be monitored for
			disconnection in order to prevent deadlocks.
*/
bool CSocketClient::WaitOnDataToBeSent(int iSlot)
{
	bool bRetVal = false;

	while (this->IsConnected() && ((CSocketServer *)this->pSockServer)->p_Clients[iSlot].IsConnected(true)
		&& (bRetVal = ((CSocketServer *)this->pSockServer)->p_Clients[iSlot].IsSendDataWaiting()) && !this->bAbortConnection)
	{
		//WaitForSingleObject(((CSocketServer *)this->pSockServer)->p_PumpCycledEvent, SSNEXTDATAINTERVAL);
		WaitForSingleObject(this->hDataSent, SSNEXTDATAINTERVAL);
	}

	return !bRetVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function will return TRUE when the client has a empty send buffer.
	Returns FALSE if the client has been disconnected or is pending a disconnect.
*/
bool CSocketClient::WaitOnDataToBeSent(void)
{
	bool bRetVal = false;

	while (this->IsConnected(true) && (bRetVal = this->IsSendDataWaiting()) && !this->bAbortConnection)
	{
		//WaitForSingleObject(((CSocketServer *)this->pSockServer)->p_PumpCycledEvent, SSNEXTDATAINTERVAL);
		WaitForSingleObject(this->hDataSent, SSNEXTDATAINTERVAL);
	}

	return !bRetVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function will return TRUE when the client has data in there receive buffer.
	Returns FALSE if the client has been disconnected or is pending a disconnect.

	NOTE: A TRUE return value does not mean that the data is a full packet,
	therfore a call to GetClientData or GetNextClientData may not return data.
*/
bool CSocketClient::WaitOnDataToBeRecvd(void)
{
	bool bRetVal = false;

	while (this->IsConnected(true) && !(bRetVal = this->IsRecvDataWaiting()) && !this->bAbortConnection)
	{
		//WaitForSingleObject(((CSocketServer *)this->pSockServer)->p_PumpCycledEvent, SSNEXTDATAINTERVAL);
		WaitForSingleObject(this->hDataReceived, SSNEXTDATAINTERVAL);
	}

	return bRetVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function will return TRUE when the receive buffer is empty.
*/
bool CSocketClient::WaitForEmptyRecvBuffer(void)
{
	bool bRetVal = false;

	while (this->IsConnected() && (bRetVal = this->IsRecvDataWaiting()) && !this->bAbortConnection)
	{
		//WaitForSingleObject(((CSocketServer *)this->pSockServer)->p_PumpCycledEvent, SSNEXTDATAINTERVAL);
		WaitForSingleObject(this->hDataSent, SSNEXTDATAINTERVAL);
	}

	return !bRetVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function will return TRUE when the client has data in there receive buffer
	and the data was a full packet.

	Returns FALSE if the client has been disconnected, is pending a disconnect or the
	data received is not a full packet.
*/

bool CSocketClient::GetNextClientData(LPBASICHUNK pChunk, LPSSWAITSTATS pWaitStats)
{
	int iSpinThreshold = SSWAITSPINCOUNT;

	do
	{
		if (this->GetClientData(pChunk))
		{
			return true;
		}
		if (pWaitStats)
		{
			pWaitStats->iTotalWaits++;
		}
		if (WaitForSingleObject(this->hDataReceived, SSNEXTDATAINTERVAL) == WAIT_TIMEOUT)
		{
			if (pWaitStats)
			{
				pWaitStats->iBeneficialWaits++;
			}
		}
		else {
			if (iSpinThreshold > 0)
			{
				iSpinThreshold--;
			}
			else {
				Sleep(1);
			}
			if (pWaitStats)
			{
				pWaitStats->iDetrimentalWaits++;
			}
		}
		//WaitForSingleObject(((CSocketServer *)this->pSockServer)->p_PumpCycledEvent, SSNEXTDATAINTERVAL);

	} while (this->IsConnected(true) && !this->bAbortConnection);

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSocketClient::GetNextClientData(LPBASICHUNK pChunk)
{
	return this->GetNextClientData(pChunk, NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to retrieve a clients data from there receive buffer.

	This function returns TRUE if the clients has data to be received,
	the CriticalSection is open, the data is a full (and valid) packet.
	Otherwise the function returns FALSE.
*/
bool CSocketClient::GetClientData(LPBASICHUNK pChunk)
{
	pChunk->iLength = 0;

	if (this->IsRecvDataWaiting())
	{
		if (!this->TryLock())
		{
			return false;
		}

		int iPacketResult = SSPACKET_UNDEFINED;

		if ((iPacketResult = ((CSocketServer *)this->pSockServer)->p_DePacketize(this, pChunk)) == SSPACKET_PARTIAL_DATA)
		{
			//We don't currently have a full packet, no big deal... it's on its way.
			this->Unlock();
			return false;
		}

		if (iPacketResult == SSPACKET_USER_FILTERED)
		{
			this->Unlock();
			return false;
		}

		if (iPacketResult <= 0)
		{
			this->bDisconnect = true;
			this->Unlock();
			return ((CSocketServer *)this->pSockServer)->p_ThrowError(1, this);
		}

		((BYTE *)pChunk->pBuffer)[pChunk->iLength] = '\0'; //The class allocates one byte ahead, so this is safe.

		if (((CSocketServer *)this->pSockServer)->pOnConsume)
		{
			if (!((CSocketServer *)this->pSockServer)->pOnConsume((CSocketServer *)this->pSockServer, this, pChunk))
			{
				this->Unlock();
				return false;
			}
		}

		this->Unlock();
		return (iPacketResult == SSPACKET_GOOD_PACKET);
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to set a clients send buffer.

	Returns TRUE if the data was set successfully.
	Returns FALSE if the CriticalSection was not free,
	the client has data waiting to be sent, or if the client
	has been disconnected or is pending a disconnect.
*/
bool CSocketClient::SetSendData(const void *pData, int iSize)
{
	//Do we already have data ready to be sent?
	if (!this->IsSendDataWaiting())
	{
		if (!this->TryLock())
		{
			return false;
		}

		//Is the client is connected and not ready to be disconnected?
		if (this->IsConnected())
		{
			//Is our clients send buffer large enough to handle the outgoing data?
			if (iSize >= this->SendChunk.iAlloc)
			{
				//Make our send buffer large enough to handle the outgoing data.
				if (!this->ReAlloc(&this->SendChunk, iSize + 1))
				{
					this->bDisconnect = true;
					this->Unlock();
					return false;
				}
			}

			//Copy the outgoing data to our send buffer.
			memcpy_s(this->SendChunk.pBuffer,
				this->SendChunk.iAlloc, pData, iSize);
			this->SendChunk.iLength = iSize;

			((BYTE *)this->SendChunk.pBuffer)[this->SendChunk.iLength] = '\0';
		}
		else {
			this->Unlock();
			return false;
		}

		this->Unlock();
		return true;
	}
	else if (!this->bPacketMode)
	{
		//Can the data be appended to the buffer?
		if ((this->SendChunk.iLength + iSize) < ((CSocketServer *)this->pSockServer)->p_iMaxBufferSize)
		{
			if (!this->TryLock())
			{
				return false;
			}

			this->ReAlloc(&this->SendChunk, (this->SendChunk.iLength + iSize));

			memcpy_s(((BYTE *)this->SendChunk.pBuffer + this->SendChunk.iLength),
				this->SendChunk.iAlloc - this->SendChunk.iLength, pData, iSize);

			this->SendChunk.iLength += iSize;

			this->Unlock();
			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to set a clients send buffer.

	Returns TRUE if the data was set successfully.
	Returns FALSE if the CriticalSection was not free,
	the client has data waiting to be sent, or if the client
	has been disconnected or is pending a disconnect.
*/
#ifdef UNICODE
bool CSocketClient::SetSendData(const WCHAR *sData)
{
	return this->SetSendData(sData, (int)wcslen(sData) * sizeof(WCHAR));
}
#else
bool CSocketClient::SetSendData(const char *sData)
{
	return this->SetSendData(sData, (int)strlen(sData));
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	See: [SetSendData(const void *pData, int iSize)] overload.
*/
bool CSocketClient::SetSendData(LPBASICHUNK pChunk)
{
	return this->SetSendData(pChunk->pBuffer, pChunk->iLength);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to set a clients send buffer.

	Returns TRUE if the data was set successfully.
	Returns FALSE if the client has been disconnected or is pending a disconnect.

	Important:
		If you are working within the ClientThreadProc() and wish to send data to a connected peer other than
			the one identified by the pClient passed into the thread proc, then you must pass the 'Slot Number'
			of the client that is to receive the message. This is because both connections must be monitored for
			disconnection in order to prevent deadlocks.
*/
bool CSocketClient::SetNextSendData(int iSlot, const void *pData, int iSize, LPSSWAITSTATS pWaitStats)
{
	int iSpinThreshold = SSWAITSPINCOUNT;

	while (this->IsConnected() && ((CSocketServer *)this->pSockServer)->p_Clients[iSlot].IsConnected(true))
	{
		if (((CSocketServer *)this->pSockServer)->p_Clients[iSlot].SetSendData(pData, iSize))
		{
			return true;
		}
		if (pWaitStats)
		{
			pWaitStats->iTotalWaits++;
		}
		if (WaitForSingleObject(this->hDataSent, SSNEXTDATAINTERVAL) == WAIT_TIMEOUT)
		{
			if (pWaitStats)
			{
				pWaitStats->iBeneficialWaits++;
			}
		}
		else {
			if (iSpinThreshold > 0)
			{
				iSpinThreshold--;
			}
			else {
				Sleep(1);
			}
			if (pWaitStats)
			{
				pWaitStats->iDetrimentalWaits++;
			}
		}		//WaitForSingleObject(((CSocketServer *)this->pSockServer)->p_PumpCycledEvent, SSNEXTDATAINTERVAL);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSocketClient::SetNextSendData(int iSlot, const void *pData, int iSize)
{
	return this->SetNextSendData(iSlot, pData, iSize, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to set a clients send buffer.

	Returns TRUE if the data was set successfully.
	Returns FALSE if the client has been disconnected or is pending a disconnect.
*/
bool CSocketClient::SetNextSendData(const void *pData, int iSize, LPSSWAITSTATS pWaitStats)
{
	int iSpinThreshold = SSWAITSPINCOUNT;

	while (this->IsConnected(true))
	{
		if (this->SetSendData(pData, iSize))
		{
			return true;
		}
		if (pWaitStats)
		{
			pWaitStats->iTotalWaits++;
		}
		if (WaitForSingleObject(this->hDataSent, SSNEXTDATAINTERVAL) == WAIT_TIMEOUT)
		{
			if (pWaitStats)
			{
				pWaitStats->iBeneficialWaits++;
			}
		}
		else {
			if (iSpinThreshold > 0)
			{
				iSpinThreshold--;
			}
			else {
				Sleep(1);
			}
			if (pWaitStats)
			{
				pWaitStats->iDetrimentalWaits++;
			}
		}
		//WaitForSingleObject(((CSocketServer *)this->pSockServer)->p_PumpCycledEvent, SSNEXTDATAINTERVAL);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CSocketClient::SetNextSendData(const void *pData, int iSize)
{
	return this->SetNextSendData(pData, iSize, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to set a clients send buffer.

	Returns TRUE if the data was set successfully.
	Returns FALSE if the client has been disconnected or is pending a disconnect.

	Important:
		If you are working within the ClientThreadProc() and wish to send data to a connected peer other than
			the one identified by the pClient passed into the thread proc, then you must pass the 'Slot Number'
			of the client that is to receive the message. This is because both connections must be monitored for
			disconnection in order to prevent deadlocks.
*/
#ifdef UNICODE
bool CSocketClient::SetNextSendData(int iSlot, const WCHAR *sData)
{
	return this->SetNextSendData(iSlot, sData, (int)wcslen(sData) * sizeof(WCHAR));
}
#else
bool CSocketClient::SetNextSendData(int iSlot, const char *sData)
{
	return this->SetNextSendData(iSlot, sData, (int)strlen(sData));
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to set a clients send buffer.

	Returns TRUE if the data was set successfully.
	Returns FALSE if the client has been disconnected or is pending a disconnect.

*/
#ifdef UNICODE
bool CSocketClient::SetNextSendData(const WCHAR *sData)
{
	return this->SetNextSendData(sData, (int)wcslen(sData) * sizeof(WCHAR));
}
#else
bool CSocketClient::SetNextSendData(const char *sData)
{
	return this->SetNextSendData(sData, (int)strlen(sData));
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	See: [SetNextSendData(const char *sData)] overload
*/
bool CSocketClient::SetNextSendData(LPBASICHUNK pChunk)
{
	return this->SetNextSendData(pChunk->pBuffer, pChunk->iLength);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	See: [SetNextSendData(const char *sData)] overload
*/
bool CSocketClient::SetNextSendData(LPBASICHUNK pChunk, LPSSWAITSTATS pWaitStats)
{
	return this->SetNextSendData(pChunk->pBuffer, pChunk->iLength, pWaitStats);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	See: [SetNextSendData(int iSlot, const char *sData)] overload.
*/
bool CSocketClient::SetNextSendData(int iSlot, LPBASICHUNK pChunk)
{
	return this->SetNextSendData(iSlot, pChunk->pBuffer, pChunk->iLength);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	See: [SetNextSendData(int iSlot, const char *sData)] overload.
*/
bool CSocketClient::SetNextSendData(int iSlot, LPBASICHUNK pChunk, LPSSWAITSTATS pWaitStats)
{
	return this->SetNextSendData(iSlot, pChunk->pBuffer, pChunk->iLength, pWaitStats);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
