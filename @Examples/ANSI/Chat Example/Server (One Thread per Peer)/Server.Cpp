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

void ClientProc(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	printf("%d Connected\n", pClient->PeerID());

	while(pClient->IsConnected())
    {
		if(pClient->GetNextClientData(pChunk))
		{
			printf("Received: [%s] (%db)\n", pChunk->pBuffer, pChunk->iLength);
			
			for(int iPeer = 0; iPeer < pSock->MaxConnections(); iPeer++)
			{
				if(pSock->Client(iPeer) != pClient) //Don't send data to the client that sent it to us.
				{
					pClient->SetNextSendData(iPeer, pChunk);
				}
			}
		}
	}

	printf("%d Disconnected\n", pClient->PeerID());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	CSocketServer Sock;

	printf("Initializing the server.\n");
	if(Sock.Initialize(1559, 25, &ClientProc, &OnError))
	{
		printf("Starting the server.\n");
		if(Sock.Start())
		{
			printf("The server is running.\n");
			while(Sock.IsActive())
			{
				Sleep(1000);
			}
			Sock.Stop();
		}
		Sock.Destroy();
	}

	system("pause");
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
