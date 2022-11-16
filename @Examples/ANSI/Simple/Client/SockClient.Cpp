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
	const int iPort = 8485;
	const int iMaxClients = 0;
	CSocketServer Sock;

	printf("Initializing the Client.\n");
	if (!Sock.Initialize(&OnError))
	{
		return 0;
	}

	//Sock.PacketMode(false);

	if (Sock.Start())
	{
		CSocketClient* pClient = NULL;

		printf("Connecting.\n");
		if (Sock.Connect("127.0.0.1", iPort, &pClient))
		{
			printf("Connected.\n");


			while (pClient->IsConnected()) //Just keep running.
			{
				printf("Sending data to server...\n");
				pClient->SetNextSendData("Hello World!");
				Sleep(100);
			}
			printf("Disconnected.\n");
		}
		else {
			printf("Failed to connect.\n");
		}
	}

	Sock.Destroy();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
