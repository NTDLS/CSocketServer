# CSocketServer
  
:loudspeaker:	A collection of classes for creating a socket server, client and communicating between them. Includes a Packetizer to solve the age old issue of TCP/IP packets being fragmented or combined in the pipe.
  
Must include WS2_32.LIB and/or WSOCK32.LIB
  
>**Name        :** OnAccept  
>**Prototype   :** bool OnAccept(CSocketServer *pSock, CSocketClient *pClient)  
>**Return      :** Return TRUE to allow the connection, FALSE to reject the connection.  
>**Description :** Called when a connection is accepted by the socket server.  
  
>**Name        :** OnAcceptConnect  
>**Prototype   :** bool OnAcceptConnect(CSocketServer *pSock, CSocketClient *pClient)  
>**Return      :** Return TRUE to allow the connection, FALSE to reject the connection.  
>**Description :** Called when a connection is accepted and/or connected by the socket server.  
  
>**Name        :** OnAfterDePacketize  
>**Prototype   :** bool OnAfterDePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)  
>**Return      :** Return TRUE to accept the packet, FALSE to reject the packet.  
>**Description :** Called after a packet is read by user code.  
                pChunk contains the actual data sent be the remote peer.  
  
>**Name        :** OnAfterPacketize  
>**Prototype   :** bool OnAfterPacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)  
>**Return      :** Return TRUE to add the packet to the senq queu, FALSE to reject the packet.  
>**Description :** Called after a packet is assembled by the socket server.  
                pChunk contains the packet data and the actual data that will by sent be the socket server.  
  
>**Name        :** OnBeforeDePacketize  
>**Prototype   :** bool OnBeforeDePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)  
>**Return      :** Return TRUE to accept the packet, FALSE to reject the packet.  
>**Description :** Called before a packet is read by user code.  
                pChunk contains the packet data and the actual data sent be the remote peer.  
  
>**Name        :** OnBeforePacketize  
>**Prototype   :** bool OnBeforePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)  
>**Return      :** Return TRUE to add the packet to the senq queu, FALSE to reject the packet.  
>**Description :** Called before a packet is assembled by the socket server.  
                pChunk contains the actual data that will be sent by the socket server.  
  
>**Name        :** OnConnect  
>**Prototype   :** bool OnConnect(CSocketServer *pSock, CSocketClient *pClient)  
>**Return      :** Return TRUE to allow the connection, FALSE to reject the connection.  
>**Description :** Called when a connection is connected by the socket server.  
  
>**Name        :** OnRecv  
>**Prototype   :** bool OnRecv(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)  
>**Return      :** Return TRUE to accept the received data, FALSE to reject.  
>**Description :** Called after a packet or partial packet is received by the socket server.  
                pChunk contains the packet data and the actual data that was received by the socket server.  
                The data passed in through pChunk is not guaranteed to be a full packet.  
  
>**Name        :** OnSend  
>**Prototype   :** bool OnSend(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)  
>**Return      :** Return TRUE to send the data, FALSE to delay the sending of the data.  
>**Description :** Called before a packet is sent by the socket server.  
                pChunk contains the packet data and the actual data that will be sent by the socket server.  
                The data passed in through pChunk is guaranteed to be a full packet.  
  
>**Name        :** OnStart  
>**Prototype   :** bool OnStart(CSocketServer *pSock, int iListenPort)  
>**Description :** Called when the socket server is started by a call to the Start() function.  
>**Return      :** Return TRUE to start the socket server, FALSE to cancel.  
  
>**Name        :** OnStop  
>**Prototype   :** bool OnStop(CSocketServer *pSock)  
>**Return      :** Return TRUE to stop the socket server, FALSE to cancel.  
>**Description :** Called when the socket server is stopped by a call to the Stop() function.  
  
>**Name        :** ClientHandlerThread  
>**Prototype   :** void ClientHandlerThread(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)  
>**Return      :** n/a  
>**Description :** Called when a client is accepted and/or connected. If this handler is set, the socket server  
                will operate in "one thread per connection" mode. This is ok for applications with few  
                simultaneous connections. If the thread exits, the client will be disconnected - Therfore  
                you should loop while the peer is connected.  
  
>**Name        :** OnBeginClientThread  
>**Prototype   :** void OnBeginClientThread(CSocketServer *pSock, CSocketClient *pClient, HANDLE hThread)  
>**Return      :** n/a  
>**Description :** Called after the a client thread is created but before the ClientHandlerThread handler is called.  
  
>**Name        :** OnDisconnect  
>**Prototype   :** void OnDisconnect(CSocketServer *pSock, CSocketClient *pClient)  
>**Return      :** n/a  
>**Description :** Called when a client has been disconnected (after the socket has been shutdown).  
  
>**Name        :** OnEndClientThread  
>**Prototype   :** void OnEndClientThread(CSocketServer *pSock, CSocketClient *pClient)  
>**Return      :** n/a  
>**Description :** Called after a client thread has been destroyed.  
  
>**Name        :** OnError  
>**Prototype   :** void OnError(CSocketServer *pSock, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg)  
>**Return      :** n/a  
>**Description :** Called for any internal exceptions.
