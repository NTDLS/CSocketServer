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

void OnError(CSocketServer *pSock, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg)
{
	if(pClient)
	{
		printf("Error: %d, Client: %d, Msg: %s\n", iErrorNumber, pClient->PeerID(), sErrorMsg);
	}
	else{
		printf("Error: %d, Msg: %s\n", iErrorNumber, sErrorMsg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ClientProc(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	printf("%d Connected.\n", pClient->PeerID());

	//Status and speed calculation functionality.
	double dBytesSent = 0, dPackets = 0, dElapsed = 0;
	clock_t ctBegin = 0, ctEnd = 0;
	int iStat = 0;

	SSWAITSTATS WaitStats;
	memset(&WaitStats, 0, sizeof(WaitStats));

	while(pClient->IsConnected())
    {
		if(pClient->GetNextClientData(pChunk, &WaitStats))
		{
			dBytesSent += pChunk->iLength;
			dPackets++;

			if((dElapsed = ((double) (clock() - ctBegin)) / CLOCKS_PER_SEC) >= 1)
			{
				printf("(%d) %.2f MB/s (%.2f Bufs/s) (Waits: %d %.2f%%)\n",
					iStat++,
					(dBytesSent/1024.0/1024.0) / dElapsed,
					dPackets/dElapsed,
					WaitStats.iTotalWaits,
					WaitStats.iTotalWaits > 0 ? ((float)WaitStats.iBeneficialWaits / (float)WaitStats.iTotalWaits) * 100.0 : 100);

				dPackets = 0;
				dBytesSent = 0;
				ctBegin = clock();
			}
		}
		//Sleep(1);
	}

	printf("%d Disconnected.\n", pClient->PeerID());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	int iPort = 8485;
	int iMaxClients = 0;
	CSocketServer Sock;

	if(CreateMutex(NULL, TRUE, "CSocketServer_SelfTest"))
	{
		if(GetLastError() == ERROR_ALREADY_EXISTS)
		{
			printf("Initializing the Client.\n");
			if(!Sock.Initialize(&ClientProc, &OnError))
			{
				return 0;
			}
		}
		else{
			char sBuf[64];
			printf("How many clients would you like to test with[1 - %d]: ", FD_SETSIZE);

			gets_s(sBuf, sizeof(sBuf));
			iMaxClients = atol(sBuf);
			if(iMaxClients >= 1 && iMaxClients <= FD_SETSIZE)
			{
				printf("Initializing the Server.\n");
				if(!Sock.Initialize(iPort, iMaxClients, &OnError))
				{
					return 0;
				}
			}
			else{
				printf("Invalid number of clients.\n");
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
				char sModule[MAX_PATH];
				GetModuleFileName(NULL, sModule, sizeof(sModule));
				ShellExecute(NULL, "OPEN", sModule, "", NULL, SW_MINIMIZE);
				Sleep(100);
			}

			//Status and speed calculation functionality.
			double dBytesSent = 0, dPackets = 0, dElapsed = 0;
			clock_t ctBegin = 0, ctEnd = 0;
						
			BASICHUNK Chunk;
			Sock.Alloc(&Chunk, 1024*1024);
			Chunk.iLength = Chunk.iAlloc;
			int iStat = 0;

			SSWAITSTATS WaitStats;
			memset(&WaitStats, 0, sizeof(WaitStats));

			ctBegin = clock();
			for(CSocketClient *pClient; pClient = Sock.GetConnectedClient();)
			{
				pClient->SetNextSendData(&Chunk, &WaitStats);

				dBytesSent += Chunk.iLength;
				dPackets++;

				if((dElapsed = ((double) (clock() - ctBegin)) / CLOCKS_PER_SEC) >= 1)
				{
					printf("(%d)  %d connections @ %.2f MB/s (%.2f Packets/s) (Waits: %d %.2f%%)\n",
						iStat++,
						Sock.CurrentConnections(),
						(dBytesSent/1024.0/1024.0) / dElapsed,
						dPackets/dElapsed,
						WaitStats.iTotalWaits,
						WaitStats.iTotalWaits > 0 ? ((float)WaitStats.iBeneficialWaits / (float)WaitStats.iTotalWaits) * 100.0 : 100
					);

					dPackets = 0;
					dBytesSent = 0;
					ctBegin = clock();
				}
				//Sleep(1);
            }

			Sock.Free(&Chunk);
		}
		else{
			printf("Connecting to the server.\n");
			
			if(!Sock.ConnectUseCurrentThread("127.0.0.1", iPort))
			{
				printf("Failed to connect to the remote server.\n");
			}
		}
	}
	else{
		printf("Failed to start the service.\n");
	}

	Sock.Destroy();
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
