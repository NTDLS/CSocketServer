#ifndef _ENTRY_CPP
#define _ENTRY_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WinSock2.H>
#include <WinSock.H>
#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

#include "../../../../CSocketServer.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OnError(CSocketServer *lpCSocketServer, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg)
{
	if(pClient)
	{
		printf("Error: %d, Client: %d, Msg: %s\n", iErrorNumber, pClient->PeerID(), sErrorMsg);
	}
	else {
		printf("Error: %d, Msg: %s\n", iErrorNumber, sErrorMsg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	CSocketServer Sock;

	printf("Initializing the server.\n");
	if(Sock.Initialize(1559, 25, &OnError))
	{
		printf("Starting the server.\n");
		if(Sock.Start())
		{
			printf("The server is running.\n");

			BASICHUNK lChunk;
			if(!Sock.Alloc(&lChunk))
			{
				printf("Failed to allocate RAM for send/recv chunk.\n");
				return 0;
			}

			int iBytes = 0;
			int iPackets = 0;

			while(Sock.IsActive())
			{
				for(int iClient = 0; iClient < Sock.MaxConnections(); iClient++)
				{
					//If the client is connected.
					if(Sock.Client(iClient)->IsConnected())
					{
						if(Sock.Client(iClient)->GetClientData(&lChunk))
						{
							iBytes += lChunk.iLength;
							iPackets++;

							printf("Packets: %d, Bytes: %d\r", iPackets, iBytes);

							//Send the received data to all of the other connected clients.
							for(int i3rdParty = 0; i3rdParty < Sock.MaxConnections(); i3rdParty++)
							{
								if(Sock.Client(i3rdParty)->IsConnected())
								{
									if(i3rdParty != iClient) //Don't send to the client that sent us the data.
									{
										Sock.Client(iClient)->SetNextSendData(i3rdParty, &lChunk);
									}
								}
							}
						}
					}
				}

				Sleep(1); //Save CPU time.
			}

			Sock.Free(&lChunk);
			Sock.Stop();
		}
		Sock.Destroy();
	}

	system("pause");
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
