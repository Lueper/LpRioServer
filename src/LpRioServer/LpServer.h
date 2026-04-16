#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

// mswsock.h보다 먼저 선언
#include <ws2tcpip.h>

#include <iostream>
#include <mswsock.h>

#include "Common/LpDefine.h"
#include "Utility/LpLogger.h"
#include "yaml-cpp/yaml.h"

struct AcceptContext {
	OVERLAPPED overlapped = {};
	SOCKET acceptSock = INVALID_SOCKET;
	char addrBuf[ADDR_LEN * 2] = { 0, };
};

class LpServer {
public:
	LpServer();
	~LpServer();

	bool Init();
	void Start();
	void Stop();
	void Release();

private:
	void Run();

	void WorkerThread();
	void OnAccept(AcceptContext* actx);
	void OnRioCompletion();
	bool PostAccept();

	SOCKET m_socket;
	HANDLE m_iocp = nullptr;
	OVERLAPPED m_overlapped = {};
	LPFN_ACCEPTEX m_lpfnAcceptEx = nullptr;
	LPFN_GETACCEPTEXSOCKADDRS m_lpfnGetAcceptExSockaddrs = nullptr;
	RIO_EXTENSION_FUNCTION_TABLE m_rio = {};
	RIO_CQ m_rioCQ = RIO_INVALID_CQ;
	RIO_BUFFERID m_recvBufId = RIO_INVALID_BUFFERID;
	RIO_BUFFERID m_sendBufId = RIO_INVALID_BUFFERID;
	char* m_recvPool = nullptr;
	char* m_sendPool = nullptr;

	std::atomic<bool> m_running;
	std::vector<std::thread*> m_ioThreadVec;
};