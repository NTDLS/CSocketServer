#ifndef _ENTRY_CPP
#define _ENTRY_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WinSock2.H>
#include <WinSock.H>
#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Time.H>

#include "../../../CSocketServer.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OnError(CSocketServer *pSock, CSocketClient *pClient, int iErrorNumber, const WCHAR *sErrorMsg)
{
	if(pClient)
	{
		wprintf(L"Error: %d, Client: %d, Msg: %s\n", iErrorNumber, pClient->PeerID(), sErrorMsg);
	}
	else{
		wprintf(L"Error: %d, Msg: %s\n", iErrorNumber, sErrorMsg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ClientProc(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	wprintf(L"%d Connected.\n", pClient->PeerID());

	//Status and speed calculation functionality.
	double dBytesSent = 0, dPackets = 0, dElapsed = 0;
	clock_t ctBegin = 0, ctEnd = 0;
					
	while(pClient->IsConnected())
    {
		if(pClient->GetNextClientData(pChunk))
		{
			dBytesSent += pChunk->iLength;
			dPackets++;

			if((dElapsed = ((double) (clock() - ctBegin)) / CLOCKS_PER_SEC) >= 1)
			{
				wprintf(L"  %.2f MB/s (%.2f Bufs/s)\n",
					(dBytesSent/1024.0/1024.0) / dElapsed,
					dPackets/dElapsed);

				dPackets = 0;
				dBytesSent = 0;
				ctBegin = clock();
			}
		}
	}

	wprintf(L"%d Disconnected.\n", pClient->PeerID());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	int iPort = 8485;
	int iMaxClients = 0;
	CSocketServer Sock;

	if(CreateMutex(NULL, TRUE, L"CSocketServer_SelfTest"))
	{
		if(GetLastError() == ERROR_ALREADY_EXISTS)
		{
			wprintf(L"Initializing the Client.\n");
			if(!Sock.Initialize(&ClientProc, &OnError))
			{
				return 0;
			}
		}
		else{
			char sBuf[64];
			wprintf(L"How many clients would you like to test with[1 - %d]: ", FD_SETSIZE);

			gets_s(sBuf, sizeof(sBuf));
			iMaxClients = atol(sBuf);
			if(iMaxClients >= 1 && iMaxClients <= FD_SETSIZE)
			{
				wprintf(L"Initializing the Server.\n");
				if(!Sock.Initialize(iPort, iMaxClients, &OnError))
				{
					return 0;
				}
			}
			else{
				wprintf(L"Invalid number of clients.\n");
				return 0;
			}
		}
	}

	if(Sock.Start())
	{
		if(Sock.IsListeningServer())
		{
			//Launch a "bunch" of client applications.
			for(int iInstance = 0; iInstance < iMaxClients; iInstance++)
			{
				WCHAR sModule[MAX_PATH];
				GetModuleFileName(NULL, sModule, sizeof(sModule));
				ShellExecute(NULL, L"OPEN", sModule, L"", NULL, SW_MINIMIZE);
				Sleep(100);
			}

			//Status and speed calculation functionality.
			double dBytesSent = 0, dPackets = 0, dElapsed = 0;
			clock_t ctBegin = 0, ctEnd = 0;
						
			BASICHUNK Chunk;
			Sock.Alloc(&Chunk, 1024*1024);
			Chunk.iLength = Chunk.iAlloc;

			ctBegin = clock();
			for(CSocketClient *pClient; pClient = Sock.GetConnectedClient();)
			{
				pClient->SetNextSendData(&Chunk);

				dBytesSent += Chunk.iLength;
				dPackets++;

				if((dElapsed = ((double) (clock() - ctBegin)) / CLOCKS_PER_SEC) >= 1)
				{
					wprintf(L"  %d connections @ %.2f MB/s (%.2f Packets/s)\n",
						Sock.CurrentConnections(),
						(dBytesSent/1024.0/1024.0) / dElapsed,
						dPackets/dElapsed);

					dPackets = 0;
					dBytesSent = 0;
					ctBegin = clock();
				}
            }

			Sock.Free(&Chunk);
		}
		else{
			wprintf(L"Connecting to the server.\n");
			
			if(!Sock.ConnectUseCurrentThread(L"127.0.0.1", iPort))
			{
				wprintf(L"Failed to connect to the remote server.\n");
			}
		}
	}
	else{
		wprintf(L"Failed to start the service.\n");
	}

	Sock.Destroy();
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
