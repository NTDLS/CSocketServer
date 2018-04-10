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

void OnError(CSocketServer *lpCSocketServer, CSocketClient *pClient, int iErrorNumber, const WCHAR *sErrorMsg)
{
	if(pClient)
	{
		wprintf(L"Error: %d, Client: %d, Msg: %s\n", iErrorNumber, pClient->PeerID(), sErrorMsg);
	}
	else {
		wprintf(L"Error: %d, Msg: %s\n", iErrorNumber, sErrorMsg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	/*
		All of this could be handled a bit more efficiently by using threads (which are built in to the class).
			If you are going to be making many simultaneous connections to the server , I would recommend using the threads.
			Otherwise, the following will work just fine.
	*/

	int iPort = 8485;
	int iMaxClients = 10;

	CSocketServer Sock;
	
	wprintf(L"Initializing the Server.\n");
	if(!Sock.Initialize(iPort, iMaxClients, &OnError))
	{
		return 0;
	}

	BASICHUNK Chunk;
	Sock.Alloc(&Chunk);

	//Sock.PacketMode(false); //This will disable packet mode for the class.

	//You can also use Sock.PacketMode(iClient, true or false); to enable/disable packetting for a single connection.
	//	this can be done at any time while the socket is connected (switch back and forth as you like depending on requirements).

	if(Sock.Start())
	{
		//Just keep running.
		while(Sock.IsActive())
		{
			//Loop through every possible client "slot".
			for(int iClient = 0; iClient < Sock.MaxConnections(); iClient++)
			{
				if(Sock.Client(iClient)->IsConnected()) //Is the client connected?
				{
					if(Sock.Client(iClient)->GetClientData(&Chunk)) //Get any pending data.
					{
						wprintf(L"Received %d bytes from client %d\n", Chunk.iLength, iClient);
						for(int iByte = 0; iByte < Chunk.iLength; iByte++)
						{
							wprintf(L"0x%x,", ((BYTE *)Chunk.pBuffer)[iByte]);
						}
						wprintf(L"\n");
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
