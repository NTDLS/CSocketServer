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

void OnError(CSocketServer *lpCSocketServer, CSocketClient *pClient, int iErrorNumber, const WCHAR *sErrorMsg)
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

void ClientProc(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	//printf("%d Connected.\n", pClient->PeerID());

	int iPackets = 0;
    while(pClient->IsConnected())
    {
		if(pClient->GetNextClientData(pChunk))
		{
			wprintf(L"Received: \"%s\" (%d bytes)\n>", pChunk->pBuffer, pChunk->iLength);
		}
	}

	printf("%d Disconnected.\n", pClient->PeerID());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	CSocketServer Sock;

	wprintf(L"Initializing the client.\n");
	//Initialize the socket server. Pass in the address to the client procedure.
	if(Sock.Initialize(&ClientProc, &OnError))
	{
		wprintf(L"Starting the client.\n");
		if(Sock.Start())
		{
			wprintf(L"Connecting to the server.\n");

			int iClient = 0;
			int iBytes = 0;
			int iPackets = 0;

			CSocketClient *pClient;

			//Connect to the server and create a new thread.
			if(Sock.ConnectAndCreateThread(L"LocalHost", 1559, &pClient))
			{
				wprintf(L"The client is running. Open more clients, then type messages!!\n\n");
				WCHAR sBuf[5120];

				wprintf(L">");

				//Keep looping while the socket is connected.
				while(pClient->IsConnected())
				{
					//Receive user data from the console.
					_getws_s(sBuf, sizeof(sBuf) / sizeof(WCHAR));

					//Send the user data.
					pClient->SetNextSendData(sBuf);
					wprintf(L"\n>");
				}
			}
			Sock.Stop();
		}
		Sock.Destroy();
	}
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
