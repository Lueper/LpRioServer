#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

// mswsock.h보다 먼저 선언
#include <ws2tcpip.h>

#include <mswsock.h>
#include <iostream>

#include "yaml-cpp/yaml.h"
#include "Utility/LpLogger.h"

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

	SOCKET m_socket;
	LPFN_ACCEPTEX m_lpfnAcceptEx = nullptr;
	LPFN_GETACCEPTEXSOCKADDRS m_lpfnGetAcceptExSockaddrs = nullptr;
	RIO_EXTENSION_FUNCTION_TABLE m_rio = {};

	std::atomic<bool> m_running;
};