///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CSocketServer_Memory_Cpp
#define _CSocketServer_Memory_Cpp
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

#ifdef _USE_GLOBAL_MEMPOOL
#include "../NSWFL/NSWFL.h"
using namespace NSWFL::Memory;
extern MemoryPool *pMem; //pMem must be defined and initalized elsewhere.

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates a chunk of RAM of specified size.
*/
bool CSocketServer::Alloc(LPBASICHUNK pChunk, int iTotalBytes)
{
	memset(pChunk, 0, sizeof(BASICHUNK));
	if (iTotalBytes > 0)
	{
		pChunk->iAlloc = iTotalBytes;
		if (!(pChunk->pBuffer = (char *) this->Alloc(pChunk->iAlloc, 1)))
		{
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates a chunk of RAM of default size.
*/
bool CSocketServer::Alloc(LPBASICHUNK pChunk)
{
	return this->Alloc(pChunk, SSALLOCATIONGRAN);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates additional RAM of specified size to a buffer.
*/
void *CSocketServer::ReAlloc(void *lpMemory, const int iSize)
{
#ifdef _USE_GLOBAL_MEMPOOL
	char *sMemory = (char *)pMem->ReAllocate(lpMemory, sizeof(char), iSize);
#else
	EnterCriticalSection(&this->p_MemoryLock);
	char *sMemory = (char *)realloc(lpMemory, iSize);
	LeaveCriticalSection(&this->p_MemoryLock);
#endif

	return sMemory;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates additional RAM of specified size to a chunk.
*/
bool CSocketServer::ReAlloc(LPBASICHUNK pChunk, int iTotalBytes)
{
	if (iTotalBytes > 0 && pChunk->iAlloc < iTotalBytes)
	{
		pChunk->iAlloc = iTotalBytes;
		if (!(pChunk->pBuffer = (char *) this->ReAlloc(pChunk->pBuffer, pChunk->iAlloc)))
		{
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates a buffer of RAM of specified size.
*/
void *CSocketServer::Alloc(const int iSize, const int iCount)
{
#ifdef _USE_GLOBAL_MEMPOOL
	char *sMemory = (char *)pMem->Allocate(iCount, iSize);
#else
	EnterCriticalSection(&this->p_MemoryLock);
	char *sMemory = (char *)calloc(iCount, iSize);
	LeaveCriticalSection(&this->p_MemoryLock);
#endif
	return sMemory;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Frees a buffer of RAM.
*/
void CSocketServer::Free(void *lpMemory)
{
#ifdef _USE_GLOBAL_MEMPOOL
	pMem->Free(lpMemory);
#else
	EnterCriticalSection(&this->p_MemoryLock);
	free(lpMemory);
	LeaveCriticalSection(&this->p_MemoryLock);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Frees a chunk of RAM.
*/
bool CSocketServer::Free(LPBASICHUNK pChunk)
{
	if (pChunk->pBuffer)
	{
		this->Free(pChunk->pBuffer);
		memset(pChunk, 0, sizeof(BASICHUNK));
		return true;
	}
	memset(pChunk, 0, sizeof(BASICHUNK));
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates a chunk of RAM of specified size.
*/
bool CSocketClient::Alloc(LPBASICHUNK pChunk, int iTotalBytes)
{
	return ((CSocketServer *)this->pSockServer)->Alloc(pChunk, iTotalBytes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates a chunk of RAM of default size.
*/
bool CSocketClient::Alloc(LPBASICHUNK pChunk)
{
	return ((CSocketServer *)this->pSockServer)->Alloc(pChunk);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates additional RAM of specified size to a buffer.
*/
void *CSocketClient::ReAlloc(void *lpMemory, const int iSize)
{
	return ((CSocketServer *)this->pSockServer)->ReAlloc(lpMemory, iSize);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates additional RAM of specified size to a chunk.
*/
bool CSocketClient::ReAlloc(LPBASICHUNK pChunk, int iTotalBytes)
{
	return ((CSocketServer *)this->pSockServer)->ReAlloc(pChunk, iTotalBytes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates a buffer of RAM of specified size.
*/
void *CSocketClient::Alloc(const int iSize, const int iCount)
{
	return ((CSocketServer *)this->pSockServer)->Alloc(iSize, iCount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Frees a buffer of RAM.
*/
void CSocketClient::Free(void *lpMemory)
{
	return ((CSocketServer *)this->pSockServer)->Free(lpMemory);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Frees a chunk of RAM.
*/
bool CSocketClient::Free(LPBASICHUNK pChunk)
{
	return ((CSocketServer *)this->pSockServer)->Free(pChunk);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
