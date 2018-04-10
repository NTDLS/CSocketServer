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
	int iPort = 8485;
	int iMaxClients = 0;
	CSocketServer Sock;

	wprintf(L"Initializing the Client.\n");
	if(!Sock.Initialize(&OnError))
	{
		return 0;
	}

	//Sock.PacketMode(false);

	if(Sock.Start())
	{
		CSocketClient *pClient = NULL;

		wprintf(L"Connecting.\n");
		if(Sock.Connect(L"127.0.0.1", iPort, &pClient))
		{
			wprintf(L"Connected.\n");

			WCHAR sMessage[100] = {0x00, 0x01, 0x02};

			//Just keep running.
			while(pClient->IsConnected())
			{
				wprintf(L"Sending %d bytes.\n", sizeof(WCHAR) * 3);
				pClient->SetNextSendData( sMessage, sizeof(WCHAR) * 3);
				Sleep(1000);
			}
			wprintf(L"Disconnected.\n");
		}
		else{
			wprintf(L"Failed to connect.\n");
		}
	}

	Sock.Destroy();
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
