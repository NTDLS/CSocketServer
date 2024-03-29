#ifndef _ENTRY_CPP
#define _ENTRY_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WinSock2.H>
#include <WinSock.H>
#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Time.H>

#include "../../../../CSocketServer.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OnError(CSocketServer* lpCSocketServer, CSocketClient* pClient, int iErrorNumber, const char* sErrorMsg)
{
	if (pClient)
	{
		printf("Error: %d, Client: %d, Msg: %s\n", iErrorNumber, pClient->PeerID(), sErrorMsg);
	}
	else {
		printf("Error: %d, Msg: %s\n", iErrorNumber, sErrorMsg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	/*
		All of this could be handled a bit more efficiently by using threads (which are built in to the class).
			If you are going to be making many simultaneous connections to the server , I would recommend using the threads.
			Otherwise, the following will work just fine.
	*/

	const int iPort = 8485;
	const int iMaxClients = 10;

	CSocketServer Sock;

	printf("Initializing the Server.\n");
	if (!Sock.Initialize(iPort, iMaxClients, &OnError))
	{
		return 0;
	}

	BASICHUNK Chunk;
	Sock.Alloc(&Chunk);

	//Sock.PacketMode(false); //This will disable packet mode for the class.

	//You can also use Sock.PacketMode(iClient, true or false); to enable/disable packetting for a single connection.
	//	this can be done at any time while the socket is connected (switch back and forth as you like depending on requirements).

	if (Sock.Start())
	{
		//Just keep running.
		while (Sock.IsActive())
		{
			//Loop through every possible client "slot".
			for (int iClient = 0; iClient < Sock.MaxConnections(); iClient++)
			{
				if (Sock.Client(iClient)->IsConnected()) //Is the client connected?
				{
					if (Sock.Client(iClient)->GetClientData(&Chunk)) //Get any pending data.
					{
						printf("Received (%d bytes) \"%s\" from client %d\n", Chunk.iLength, (char*)Chunk.pBuffer, iClient);
					}
				}
			}

			Sleep(1);
		}
	}

	Sock.Free(&Chunk);

	Sock.Destroy();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
