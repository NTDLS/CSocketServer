///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_Events_Cpp
#define _CSocketServer_Events_Cpp
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
/* Sets an "Event Handler":

	Called when a client is accepted and/or connected. If this handler is set, the socket server
		will operate in "one thread per connection" mode. This is ok for applications with few
		simultaneous connections. If the thread exits, the client will be disconnected - Therfore
		you should loop while the peer is connected.
*/
void CSocketServer::SetClientThread(ClientHandlerThread lpClientThread)
{
	this->p_pClientHandlerThread = lpClientThread;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Return	    : n/a
	Description : Called before a packet is assembled by the socket server.
					pChunk contains the actual data that will be sent by the socket server.
*/
void CSocketServer::SetOnBeforePacketize(OnBeforePacketize lpOnBeforePacketize)
{
	this->pOnBeforePacketize = lpOnBeforePacketize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Return	    : n/a
	Description : Called after a packet is assembled by the socket server.
					pChunk contains the packet data and the actual data that will by sent be the socket server.

*/
void CSocketServer::SetOnAfterPacketize(OnAfterPacketize lpOnAfterPacketize)
{
	this->pOnAfterPacketize = lpOnAfterPacketize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Return	    : n/a
	Description : Called before a packet is read by user code.
					pChunk contains the packet data and the actual data sent be the remote peer.
*/
void CSocketServer::SetOnBeforeDePacketize(OnBeforeDePacketize lpOnBeforeDePacketize)
{
	this->pOnBeforeDePacketize = lpOnBeforeDePacketize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Return	    : n/a
	Description : Called after a packet is read by user code.
				pChunk contains the actual data sent be the remote peer.
*/
void CSocketServer::SetOnAfterDePacketize(OnAfterDePacketize lpOnAfterDePacketize)
{
	this->pOnAfterDePacketize = lpOnAfterDePacketize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Return	    : n/a
	Description : Called at the end of every complete iteration of the pump thread.
*/
void CSocketServer::SetOnPump(OnPump lpOnPump)
{
	this->pOnPump = lpOnPump;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Return	    : n/a
	Description : Called at for every connected peer when at each iteration of the pump thread.
*/
void CSocketServer::SetOnClientPump(OnClientPump lpOnClientPump)
{
	this->pOnClientPump = lpOnClientPump;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Called for any internal exceptions.
*/
void CSocketServer::SetOnError(OnError lpOnError)
{
	this->pOnError = lpOnError;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Called after the a client thread is created but before the ClientHandlerThread handler is called.
*/
void CSocketServer::SetOnBeginClientThread(OnBeginClientThread lpOnBeginClientThread)
{
	this->pOnBeginClientThread = lpOnBeginClientThread;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Called after a client thread has been destroyed.
*/
void CSocketServer::SetOnEndClientThread(OnEndClientThread lpOnEndClientThread)
{
	this->pOnEndClientThread = lpOnEndClientThread;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Return	  : Return TRUE to allow the connection, FALSE to reject the connection.
	Description : Called when a connection is accepted and/or connected by the socket server.
*/
void CSocketServer::SetOnAcceptConnect(OnAcceptConnect lpOnAcceptConnect)
{
	this->pOnAcceptConnect = lpOnAcceptConnect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Return	  : Return TRUE to allow the connection, FALSE to reject the connection.
	Description : Called when a connection is accepted by the socket server.
*/
void CSocketServer::SetOnAccept(OnAccept lpOnAccept)
{
	this->pOnAccept = lpOnAccept;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Return	  : Return TRUE to allow the connection, FALSE to reject the connection.
	Description : Called when a connection is connected by the socket server.
*/
void CSocketServer::SetOnConnect(OnConnect lpOnConnect)
{
	this->pOnConnect = lpOnConnect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Called when a client has been disconnected (after the socket has been shutdown).
*/
void CSocketServer::SetOnDisconnect(OnDisconnect lpOnDisconnect)
{
	this->pOnDisconnect = lpOnDisconnect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Description : Called when the socket server is started by a call to the Start() function.
	Return	  : Return TRUE to start the socket server, FALSE to cancel.
*/
void CSocketServer::SetOnStart(OnStart lpOnStart)
{
	this->pOnStart = lpOnStart;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Return	  : Return TRUE to stop the socket server, FALSE to cancel.
	Description : Called when the socket server is stopped by a call to the Stop() function.
*/
void CSocketServer::SetOnStop(OnStop lpOnStop)
{
	this->pOnStop = lpOnStop;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Return	  : Return TRUE to send the data, FALSE to delay the sending of the data.
	Description : Called before a packet is sent by the socket server.
					pChunk contains the packet data and the actual data that will be sent by the socket server.
					The data passed in through pChunk is guaranteed to be a full packet.
*/
void CSocketServer::SetOnSend(OnSend lpOnSend)
{
	this->pOnSend = lpOnSend;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Return	  : Return TRUE to accept the received data, FALSE to reject.
	Description : Called after a packet or partial packet is received by the socket server.
					pChunk contains the packet data and the actual data that was received by the socket server.
					The data passed in through pChunk is not guaranteed to be a full packet.
*/
void CSocketServer::SetOnRecv(OnRecv lpOnRecv)
{
	this->pOnRecv = lpOnRecv;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Sets an "Event Handler":

	Return	    : Return TRUE to accept the received data, FALSE to reject.
	Description : Called after data is removed from the receive buffer.
					pChunk contains the packet data and the actual data that was received by the socket server.
					The data passed in through pChunk is guaranteed to be a full packet.
*/
void CSocketServer::SetOnConsume(OnConsume lpOnConsume)
{
	this->pOnConsume = lpOnConsume;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
