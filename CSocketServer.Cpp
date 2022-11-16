///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_Cpp
#define _CSocketServer_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WinSock2.H>
#include <WinSock.H>
#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Time.H>

#include "CSocketServer.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef UNICODE
const WCHAR *sSocketServerError[] = {
	L"OK",																			//0
	L"Received malformed packet.",													//1
	L"Connection expired due to inactivity.",										//2
	L"Accept failed: Socket error.",												//3
	L"Accept failed: Pre-accept disconnection.",									//4
	L"Accept failed: Too many clients.",											//5
	L"Accept failed: Not enough resources.",										//6
	L"Accept failed: Could not initialize client.",									//7
	L"Accept failed: Rejected by filter.",											//8
	L"WSAStartup failed.",															//9
	L"WSACleanup failed.",															//10
	L"Winsock buffer is full.",														//11
	L"CreateThread failed.",														//12
	L"Memory Allocation Error.",													//13
	L"GetExitCodeThread failed.",													//14
	L"Possible thread dead-lock. Thread will be terminated.",						//15
	L"Socket failed.",																//16
	L"Bind failed. Port already in use?",											//17
	L"Listen failed.",																//18
	L"OpenListenSocket failed.",													//19
	L"StartTcpPump failed.",														//20
	L"StopTcpPump failed.",															//21
	L"CloseListenSocket failed.",													//22
	L"CreateThread failed.",														//23
	L"GetExitCodeThread failed.",													//24
	L"Socket select error.",														//25
	L"Connect failed: Too many clients.",											//26
	L"Connect failed: Not enough resources.",										//27
	L"Connect failed: Could not initialize client.",								//28
	L"Maximum connections is set too high. Reduce value or increse FD_SETSIZE.",	//29
	L"Failed to resolve host name.",												//30
	L"Connect failed: Rejected by filter.",											//31
	L"Maximum connections is set too high. Check value of SocketLimit().",			//32
	L"Connect failed: General connection failure.",									//33
	NULL
};
#else
const char *sSocketServerError[] = {
	"OK",																		//0
	"Received malformed packet.",												//1
	"Connection expired due to inactivity.",									//2
	"Accept failed: Socket error.",												//3
	"Accept failed: Pre-accept disconnection.",									//4
	"Accept failed: Too many clients.",											//5
	"Accept failed: Not enough resources.",										//6
	"Accept failed: Could not initialize client.",								//7
	"Accept failed: Rejected by filter.",										//8
	"WSAStartup failed.",														//9
	"WSACleanup failed.",														//10
	"Winsock buffer is full.",													//11
	"CreateThread failed.",														//12
	"Memory Allocation Error.",													//13
	"GetExitCodeThread failed.",												//14
	"Possible thread dead-lock. Thread will be terminated.",					//15
	"Socket failed.",															//16
	"Bind failed. Port already in use?",										//17
	"Listen failed.",															//18
	"OpenListenSocket failed.",													//19
	"StartTcpPump failed.",														//20
	"StopTcpPump failed.",														//21
	"CloseListenSocket failed.",												//22
	"CreateThread failed.",														//23
	"GetExitCodeThread failed.",												//24
	"Socket select error.",														//25
	"Connect failed: Too many clients.",										//26
	"Connect failed: Not enough resources.",									//27
	"Connect failed: Could not initialize client.",								//28
	"Maximum connections is set too high. Reduce value or increse FD_SETSIZE.",	//29
	"Failed to resolve host name.",												//30
	"Connect failed: Rejected by filter.",										//31
	"Maximum connections is set too high. Check value of SocketLimit().",		//32
	"Connect failed: General connection failure.",								//33
	"Failed to set socket option.",												//34
	NULL
};
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CSOCKETSERVER_EXPORTS
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
