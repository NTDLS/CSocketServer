///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_H
#define _CSocketServer_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CSOCKETSERVER_EXPORTS
#define CSSAPI __declspec(dllexport)
//#elif CSOCKETSERVER_IMPORTS
//#define CSSAPI __declspec(dllimport)
#else
#define CSSAPI
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SSDFLTINITBUFSZ		65535	// Default initial size of the common pump buffer.
#define SSDFLTMAXBUFSZ		65535	// Default maximum initial size of the common pump buffer.
#define SSALLOCATIONGRAN	1024	// Granularity of internal reallocations.
#define SSTHREADTIMEOUT		10000	// Number of milliseconds to wait for a thread to start or stop.
#define SSSTOPFORCETIMEOUT	5000	// Number of milliseconds to wait for clients to disconnect on destroy.
#define SSPACKETHEADERSZ	8		// Size of the packet header (in bytes).
#define SSNEXTDATAINTERVAL	1		// Max amount of milliseconds to wait on pump thread before polling buffer state.
#define SSPACKETHEADER		985746	// Default packet header.
#define SSWAITSPINCOUNT		1000	// The number of time threads should spin-wait before resting.

//Packet results:
#define SSPACKET_GOOD_PACKET     1	// OK.
#define SSPACKET_PARTIAL_DATA    2	// OK.
#define SSPACKET_UNDEFINED       0	// Error.
#define SSPACKET_INVALID_HEADER -2	// Error.
#define SSPACKET_INVALID_PACKET -3	// Error.
#define SSPACKET_MEMORY_ERROR   -4	// Error.
#define SSPACKET_USER_FILTERED  -5	// Error.

#ifdef UNICODE
extern const WCHAR *sSocketServerError[];
#else
extern const char *sSocketServerError[];
#endif

#ifdef _DEBUG
#define MS_VC_EXCEPTION 0x406D1388
typedef struct tag_THREADNAME_INFO {
	DWORD dwType;		// Must be 0x1000.
	LPCSTR szName;		// Pointer to name (in user address space).
	DWORD dwThreadID;	// Thread ID (-1 = caller thread).
	DWORD dwFlags;		// Reserved for future use, must be zero.
} THREADNAME_INFO;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Wait_Stats {
	int iTotalWaits;
	int iDetrimentalWaits;
	int iBeneficialWaits;
} SSWAITSTATS, *LPSSWAITSTATS;

typedef struct _tag_Basic_Chunk {
	void *pBuffer;
	int iLength;
	int iAlloc;
} BASICHUNK, *LPBASICHUNK;

typedef struct _tag_Listen_Address {
	char IP[16]; //IPv4
} LISTENADDRESS, *LPLISTENADDRESS;

typedef struct _tag_Listen_Addresses {
	int Count;
	LISTENADDRESS *Address;
} LISTENADDRESSES, *LPLISTENADDRESSES;

typedef struct _tag_Listen_Sockets {
	int Count;
	SOCKET *Socket;
} LISTENSOCKETS, *LPLISTENSOCKETS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSSAPI CSocketClient {

public:
	bool bConnected;			// Remote client's connection flag.
	bool bDisconnect;			// Remote client's disconnect flag.
	bool bWasConnect;			// True is the client was connected, false if the client was accepted.
	bool bReserved;				// Reserved client slots will not be given out even if disconnected.
	bool bPacketMode;			// Is the client in packet mode?
	bool bAbortConnection;		// Used internally by the disconnection routine to exit all waiting "sends" and "receives".

#ifdef UNICODE
	WCHAR sPeerAddress[64];		// The IP address of the connected Peer.
#else
	char sPeerAddress[64];		// The IP address of the connected Peer.
#endif

	int iPeerPort;				// The IP port of the connected Peer.
	int iPeerID;				// Remote client's ID (Just a sequential number per connection for unique client identification).
	int iSlot;					// The slot that this (client class) occupies in the parent class.

	BASICHUNK SendChunk;		// Client's send buffer.
	BASICHUNK RecvChunk;		// Client's receive buffer.

	SOCKET Socket;				// Remote client's socket.
	HANDLE hClientThread;		// The client thread Handle.
	HANDLE hThreadInitEvent;	// Event wait handle used for initializing the thread.
	HANDLE hDataReceived;		// Event handle for efficient thread waiting on data arrival.
	HANDLE hDataSent;			// Event handle for efficient thread waiting on data departure.
	time_t ttLastActivity;		// Used to keep track of client activity.

	VOID *pUserData;			// Generic pointer for storing data per connection (feel free to use).

	VOID *pSockServer;			// Pointer to the "parent" socket server class.

public: //Helper functions for sending & receiving "Commands"
#ifdef UNICODE
	bool SendCommand(const WCHAR *sCommand);
	bool SendCommand(const WCHAR *sCommand, const WCHAR *sData);
	bool SendCommand(const WCHAR *sCommand, const void *pData, const int iDataLength);
	bool SendCommand(const WCHAR *sCommand, const void *pData, const int iDataLength, BASICHUNK *pTempChunk);
	bool SendCommand(const WCHAR *sCommand, const void *pData, const int iDataSz, void *pTemp, int iMaxTempSize);
#else
	bool SendCommand(const char *sCommand);
	bool SendCommand(const char *sCommand, const char *sData);
	bool SendCommand(const char *sCommand, const void *pData, const int iDataLength);
	bool SendCommand(const char *sCommand, const void *pData, const int iDataLength, BASICHUNK *pTempChunk);
	bool SendCommand(const char *sCommand, const void *pData, const int iDataSz, void *pTemp, int iMaxTempSize);
#endif

public: //General public functions.
	bool GetClientData(LPBASICHUNK pChunk);
	bool GetNextClientData(LPBASICHUNK pChunk);
	bool GetNextClientData(LPBASICHUNK pChunk, LPSSWAITSTATS pWaitStats);

	bool SetNextSendData(const void *pData, int iSize);
#ifdef UNICODE
	bool SetNextSendData(const WCHAR *sData);
#else
	bool SetNextSendData(const char *sData);
#endif
	bool SetNextSendData(LPBASICHUNK pChunk);
	bool SetNextSendData(LPBASICHUNK pChunk, LPSSWAITSTATS pWaitStats);

	bool WaitOnDataToBeSent(int iTargetClientID);
	bool SetNextSendData(int iTargetClientID, const void *pData, int iSize);
	bool SetNextSendData(const void *pData, int iSize, LPSSWAITSTATS pWaitStats);
	bool SetNextSendData(int iSlot, const void *pData, int iSize, LPSSWAITSTATS pWaitStats);

#ifdef UNICODE
	bool SetNextSendData(int iTargetClientID, const WCHAR *sData);
#else
	bool SetNextSendData(int iTargetClientID, const char *sData);
#endif
	bool SetNextSendData(int iTargetClientID, LPBASICHUNK pChunk);
	bool SetNextSendData(int iSlot, LPBASICHUNK pChunk, LPSSWAITSTATS pWaitStats);

#ifdef UNICODE
	bool SetSendData(const WCHAR *sData);
#else
	bool SetSendData(const char *sData);
#endif
	bool SetSendData(const void *pData, int iSize);
	bool SetSendData(LPBASICHUNK pChunk);

	bool IsRecvDataWaiting(void);
	bool IsSendDataWaiting(void);

	bool WaitOnDataToBeRecvd(void);
	bool WaitOnDataToBeSent(void);
	bool WaitForEmptyRecvBuffer(void);

	bool ResumeClientThread(void);
	bool PauseClientThread(void);

	void Reserved(bool bReserve);
	bool Reserved(void);

	bool WasConnect(void);

	bool PacketMode(void);
	void PacketMode(bool bUsePacketing);

	HANDLE ThreadHandle(void);
	HANDLE EventDataReceivedWaitHandle(void);
	HANDLE EventDataSentWaitHandle(void);

	int PeerID(void);
	int Slot(void);

#ifdef UNICODE
	WCHAR *PeerAddress(void);
	int PeerAddress(WCHAR *sOutAddress, int iMaxSz);
#else
	char *PeerAddress(void);
	int PeerAddress(char *sOutAddress, int iMaxSz);
#endif
	int PeerPort(void);

	VOID *UserData(void);
	void UserData(VOID *pData);

	bool IsDisconnectSet(void);
	bool IsConnected(void);
	bool IsConnected(bool bIgnoreReserved);
	bool Disconnect(void);

	bool TryLock(void);
	void Lock(void);
	void Unlock(void);

public: //Internal memory handing (**Extremely** thread safe):
	bool Alloc(LPBASICHUNK pChunk);
	bool Alloc(LPBASICHUNK pChunk, int iTotalBytes);
	void *Alloc(const int iSize, const int iCount);

	bool ReAlloc(LPBASICHUNK pChunk, int iTotalBytes);
	void *ReAlloc(void *lpMemory, const int iSize);

	bool Free(LPBASICHUNK pChunk);
	void Free(void *lpMemory);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CSSAPI CSocketServer {

	friend CSocketClient;

public: //Delegates:
	typedef bool(*OnAccept)(CSocketServer *pSock, CSocketClient *pClient);
	typedef bool(*OnAcceptConnect)(CSocketServer *pSock, CSocketClient *pClient);
	typedef bool(*OnAfterDePacketize)(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	typedef bool(*OnAfterPacketize)(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	typedef bool(*OnBeforeDePacketize)(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	typedef bool(*OnBeforePacketize)(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	typedef bool(*OnConnect)(CSocketServer *pSock, CSocketClient *pClient);
	typedef bool(*OnConsume)(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	typedef bool(*OnRecv)(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	typedef bool(*OnSend)(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	typedef bool(*OnStart)(CSocketServer *pSock, int iListenPort);
	typedef bool(*OnStop)(CSocketServer *pSock);
	typedef void(*ClientHandlerThread)(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	typedef void(*OnBeginClientThread)(CSocketServer *pSock, CSocketClient *pClient, HANDLE hThread);
	typedef void(*OnDisconnect)(CSocketServer *pSock, CSocketClient *pClient);
	typedef void(*OnEndClientThread)(CSocketServer *pSock, CSocketClient *pClient);
	typedef void(*OnPump)(CSocketServer *pSock);
	typedef void(*OnClientPump)(CSocketServer *pSock, CSocketClient *pClient);

#ifdef UNICODE
	typedef void(*OnError)(CSocketServer *pSock, CSocketClient *pClient, int iErrorNumber, const WCHAR *sErrorMsg);
#else
	typedef void(*OnError)(CSocketServer *pSock, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg);
#endif

public: //Internal memory handing (**Extremely** thread safe):
	bool Alloc(LPBASICHUNK pChunk);
	bool Alloc(LPBASICHUNK pChunk, int iTotalBytes);
	void *Alloc(const int iSize, const int iCount);

	bool ReAlloc(LPBASICHUNK pChunk, int iTotalBytes);
	void *ReAlloc(void *lpMemory, const int iSize);

	bool Free(LPBASICHUNK pChunk);
	void Free(void *lpMemory);

public: //Helper functions for sending & receiving "Commands"

	int BreakDataFromCommand(const void *pBuffer, const int iBufferLength,
		const int iCommandLength, void *pOutBuffer, int iMaxOutBufferSz);

#ifdef UNICODE
	bool IsCommand(LPBASICHUNK lpInCmd, const WCHAR *sCommand);
	bool ParseCommand(LPBASICHUNK lpInCmd, const WCHAR *sCommand, LPBASICHUNK lpOutCmd);
	bool ParseCommand(LPBASICHUNK lpInOutCmd, const WCHAR *sCommand);
	int CompareCommand(const WCHAR *sBuffer, const int iInitialBufferSize, const WCHAR *sCommand);
	int CompareCommand(LPBASICHUNK lpInOutCmd, const WCHAR *sCommand);
	int BreakDataFromCommand(LPBASICHUNK lpInCmd, const int iCommandLength, LPBASICHUNK lpOutData);
	bool CompareCommandAndBreakData(LPBASICHUNK lpInCmd, const WCHAR *sCommand, LPBASICHUNK lpOutData);
#else
	bool IsCommand(LPBASICHUNK lpInCmd, const char *sCommand);
	bool ParseCommand(LPBASICHUNK lpInCmd, const char *sCommand, LPBASICHUNK lpOutCmd);
	bool ParseCommand(LPBASICHUNK lpInOutCmd, const char *sCommand);
	int CompareCommand(const char *sBuffer, const int iInitialBufferSize, const char *sCommand);
	int CompareCommand(LPBASICHUNK lpInOutCmd, const char *sCommand);
	int BreakDataFromCommand(LPBASICHUNK lpInCmd, const int iCommandLength, LPBASICHUNK lpOutData);
	bool CompareCommandAndBreakData(LPBASICHUNK lpInCmd, const char *sCommand, LPBASICHUNK lpOutData);
#endif

public: //General public functions.

	CSocketClient *Client(int iSlot);
	CSocketClient *Peer(int iPeerID);

	bool GetNextConnectedClient(int *iSlot);
	CSocketClient *GetNextConnectedClient(void);

	bool GetConnectedClient(int *iSlot);
	CSocketClient *GetConnectedClient(void);

	int Bin2Hex(void* lpTarget, int iMaxSz, const void* lpSrc, int iSz, int iBytes, bool bPrefix);
	int Bin2Hex(void* lpTarget, int iMaxSz, const void* lpSrc, int iSz, int iBytes);
	bool NewGUID(char* sOut, int iMaxOutSz);

#ifdef UNICODE
	bool IsIPAddress(const WCHAR *sString);
	bool ResolveHost(const WCHAR *sHostName, WCHAR *sOutIP, int iMaxOutSz);
#else
	bool IsIPAddress(const char *sString);
	bool ResolveHost(const char *sHostName, char *sOutIP, int iMaxOutSz);
#endif

#ifdef UNICODE
	bool Connect(const WCHAR *sHostOrIP, int iRemotePort, CSocketClient **pOutClient);
	bool ConnectByHost(const WCHAR *sHostName, int iRemotePort, CSocketClient **pOutClient);
	bool ConnectByIP(const WCHAR *sIPAddress, int iRemotePort, CSocketClient **pOutClient);
	bool ConnectAndCreateThread(const WCHAR *sHostOrIP, int iRemotePort, CSocketClient **pOutClient);
	bool ConnectByHostAndCreateThread(const WCHAR *sHostName, int iRemotePort, CSocketClient **pOutClient);
	bool ConnectByIPAndCreateThread(const WCHAR *sIPAddress, int iRemotePort, CSocketClient **pOutClient);
	bool ConnectUseCurrentThread(const WCHAR *sHostOrIP, int iRemotePort);
	bool ConnectByHostUseCurrentThread(const WCHAR *sHostName, int iRemotePort);
	bool ConnectByIPUseCurrentThread(const WCHAR *sIPAddress, int iRemotePort);
#else
	bool Connect(const char *sHostOrIP, int iRemotePort, CSocketClient **pOutClient);
	bool ConnectByHost(const char *sHostName, int iRemotePort, CSocketClient **pOutClient);
	bool ConnectByIP(const char *sIPAddress, int iRemotePort, CSocketClient **pOutClient);
	bool ConnectAndCreateThread(const char *sHostOrIP, int iRemotePort, CSocketClient **pOutClient);
	bool ConnectByHostAndCreateThread(const char *sHostName, int iRemotePort, CSocketClient **pOutClient);
	bool ConnectByIPAndCreateThread(const char *sIPAddress, int iRemotePort, CSocketClient **pOutClient);
	bool ConnectUseCurrentThread(const char *sHostOrIP, int iRemotePort);
	bool ConnectByHostUseCurrentThread(const char *sHostName, int iRemotePort);
	bool ConnectByIPUseCurrentThread(const char *sIPAddress, int iRemotePort);
#endif

	double MaxInactivityTime(void);
	void MaxInactivityTime(double dSeconds);

	void AllowNewConnections(bool bAllow);

	void PacketMode(bool bUsePacketing);
	bool PacketMode(void);

	bool PausePumpThread(void);
	bool ResumePumpThread(void);

	int GetBufferSize(void);

	int PacketHeader(void);
	void PacketHeader(int iPacketHeader);

	void NextPeerID(int iPeerID);
	int NextPeerID(void);

	int ListenPort(void);
	int ListenPort(int iNewPort);
	void InitialBufferSize(int iSize);
	int InitialBufferSize(void);
	void MaxBufferSize(int iSize);
	int MaxBufferSize(void);

	VOID *UserData(void);
	void UserData(VOID *pData);

	int CurrentConnections(void);
	int MaxConnections(void);

	bool OptimizeForThroughput(bool bValue);
	bool OptimizeForThroughput(void);

	bool ForceSocketReuse(bool bValue);
	bool ForceSocketReuse(void);

	bool IsListeningServer(void);
	bool IsActive(void);
	bool IsInitialized(void);

	bool Start(void);
	bool Start(int iListenPort);
	bool Stop(void);
	bool Stop(bool bForce);

	bool AddListenAddress(const char *sIPv4);
	void ClearListenAddresses(void);

	bool Destroy(void);

	bool Initialize(int iListenPort, int iMaxConnections, OnError lpOnError); //Low level init.
	bool Initialize(int iListenPort, int iMaxConnections, ClientHandlerThread lpClientThread);
	bool Initialize(int iListenPort, int iMaxConnections, ClientHandlerThread lpClientThread, OnError lpOnError);
	bool Initialize(OnError lpOnError);
	bool Initialize(ClientHandlerThread lpClientThread);
	bool Initialize(ClientHandlerThread lpClientThread, OnError lpOnError);

	bool TryLock(void);
	void Lock(void);
	void Unlock(void);

private: //Internal events.
	OnPump pOnPump;
	OnClientPump pOnClientPump;
	OnAccept pOnAccept;
	OnAcceptConnect pOnAcceptConnect;
	OnAfterDePacketize pOnAfterDePacketize;
	OnAfterPacketize pOnAfterPacketize;
	OnBeforeDePacketize pOnBeforeDePacketize;
	OnBeforePacketize pOnBeforePacketize;
	OnBeginClientThread pOnBeginClientThread;
	OnConnect pOnConnect;
	OnDisconnect pOnDisconnect;
	OnEndClientThread pOnEndClientThread;
	OnError pOnError;
	OnRecv pOnRecv;
	OnConsume pOnConsume;
	OnSend pOnSend;
	OnStart pOnStart;
	OnStop pOnStop;

public: //Public events.
	void SetOnPump(OnPump lpOnPump);
	void SetOnClientPump(OnClientPump lpOnClientPump);
	void SetClientThread(ClientHandlerThread lpClientThread);
	void SetOnAccept(OnAccept lpOnAccept);
	void SetOnAcceptConnect(OnAcceptConnect lpOnAcceptConnect);
	void SetOnAfterDePacketize(OnAfterDePacketize lpOnAfterDePacketize);
	void SetOnAfterPacketize(OnAfterPacketize lpOnAfterPacketize);
	void SetOnBeforeDePacketize(OnBeforeDePacketize lpOnBeforeDePacketize);
	void SetOnBeforePacketize(OnBeforePacketize lpOnBeforePacketize);
	void SetOnBeginClientThread(OnBeginClientThread lpOnBeginClientThread);
	void SetOnConnect(OnConnect lpOnConnect);
	void SetOnDisconnect(OnDisconnect lpOnDisconnect);
	void SetOnEndClientThread(OnEndClientThread lpOnEndClientThread);
	void SetOnError(OnError lpOnError);
	void SetOnConsume(OnConsume lpOnConsume);
	void SetOnRecv(OnRecv lpOnRecv);
	void SetOnSend(OnSend lpOnSend);
	void SetOnStart(OnStart lpOnStart);
	void SetOnStop(OnStop lpOnStop);

private: //Various internal functions.
	bool p_CloseListenSocket(void);
	void p_DisconnectClient(CSocketClient *pClient);
	bool p_Accept(SOCKET listenSocket);
#ifdef UNICODE
	bool p_Connect(const WCHAR *sIPAddress, int iRemotePort, CSocketClient **pOutClient, bool bCreateThread, bool bUseCurrentThread);
#else
	bool p_Connect(const char *wsIPAddress, int iRemotePort, CSocketClient **pOutClient, bool bCreateThread, bool bUseCurrentThread);
#endif
	bool p_Receive(CSocketClient *pClient);
	bool p_Send(CSocketClient *pClient);
	bool p_InitializeClientVariables(CSocketClient *pClient);
	bool p_OpenListenSocket(int iListenPort);
	bool p_StartClientThread(CSocketClient *pClient);
	bool p_StartWindowsSocket(void);
	bool p_StopWindowsSocket(void);
	bool p_ThrowError(int iErrorNumber);
	bool p_ThrowError(int iErrorNumber, CSocketClient *pClient);
	bool p_WaitOnClientThreadToExit(CSocketClient *pClient);
	int p_AnalysePacket(CSocketClient *pClient);
	int p_DePacketize(CSocketClient *pClient, BASICHUNK *pChunk);
	CSocketClient *p_GetFreeSocketSlot(int *pOutSlot);	// Search for free client "Slot".
	int p_Packetize(CSocketClient *pClient);
	void p_ResetClientSlot(CSocketClient *pClient);

#ifdef _DEBUG
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	void p_SetThreadName(const char *sThreadName);
#endif
#endif

public:
	VOID *pUserData;	// Generic pointer for storing data (feel free to use).

private: //Various internal variables.
	bool p_bActive;					// Is the server started?
	bool p_bIsInitialized;			// is the class initialized?
	bool p_bAllowNewConnections;	// Does the class allow new connection (Accept / Connect)?
	bool p_bForceSocketReuse;		// Forces the socket server to reuse a socket even after binding errors.
	bool p_bOptimizeForThroughput;	// Optimizing for throughput will allow the socket server to use additional CPU time to aid in high throughput rates.
	int p_iListenPort;				// The port the server was set to listen on (if any).
	HANDLE p_PumpCycledEvent;		// The pump thread has completed a cycle.

	LISTENADDRESSES p_ListenAddrs;  // The IP addresses to listen on (of all if not set).

	CSocketClient *p_Clients;		// The Conneced Client Information structure.

	int p_iInitialBufferSize;       // The initial buffer size, the buffer can be resized at any time.
	int p_iMaxBufferSize;           // The max (with a bit of play) that the buffer can be reseized to.
	int p_iPacketHeader;			// The packet header that will be used with packeting.
	int p_iCurrentClients;			// The number of clients which are connected to the server.
	int p_iMaxConnections;			// Max clients allowed to connect.
	int p_iNextPeerID;				// The next assigned Client ID.
	int p_iSocketLimit;				// The maximum number of sockets allowed by winsock.
	double p_dMaxInactivityTime;	// The maximum amount of time (in seconds) that a peer is alowed to be idle.
	BASICHUNK p_Buffer;				// The class buffer.

	LISTENSOCKETS p_ListenSockets;  // Listen socket(s) of local server.

	bool p_bIsListeningServer;		// Is this class acting as a listening server?
	bool p_bPacketMode;				// Is the class using "Packet Mode" or "Raw Mode"?

	//------------------------------------------------------------------------
	//The TcpPump routine threading items (Begin)
	//------------------------------------------------------------------------
	friend unsigned __stdcall p_TcpPumpThreadProc(void *pData);

	unsigned int p_TCPPump(void);
	bool p_StartTcpPump(void);
	bool p_StopTcpPump(void);

	bool p_bShutdownTcpPump;
	bool p_bTcpPumpActive;
	HANDLE p_hTcpPumpEvent;
	HANDLE p_hTcpPump;
	//------------------------------------------------------------------------
	//The TcpPump routine threading items (End)
	//------------------------------------------------------------------------

	CRITICAL_SECTION p_TcpPumpLock;
	CRITICAL_SECTION p_HostResolverLock;
#ifndef _USE_GLOBAL_MEMPOOL
	CRITICAL_SECTION p_MemoryLock;
#endif

	//The client thread function and delegate.
	friend unsigned __stdcall p_CilentThreadProc(void *pData);
	ClientHandlerThread p_pClientHandlerThread;
};

// Used to pass class data to the client thread
typedef struct _CLIENTTHREADSTARTUPINFO {
	CSocketServer *pSock;
	CSocketClient *pClient;
	bool bIsNewThread;
}CLIENTTHREADSTARTUPINFO, *LPCLIENTTHREADSTARTUPINFO;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
