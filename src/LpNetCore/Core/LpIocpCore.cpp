#include "pch.h"

#include "LpNetCore.h"

LPFN_ACCEPTEX				LpIocpCore::AcceptEx = nullptr;
LPFN_GETACCEPTEXSOCKADDRS	LpIocpCore::GetAcceptExSockaddrs = nullptr;
LPFN_CONNECTEX				LpIocpCore::ConnectEx = nullptr;
LPFN_DISCONNECTEX			LpIocpCore::DisconnectEx = nullptr;

LpIocpCore::LpIocpCore() {
	Startup();
	std::cout << "Iocp 생성" << "\n";
}

LpIocpCore::~LpIocpCore() {
	Cleanup();
	std::cout << "Iocp 소멸" << "\n";
}

bool LpIocpCore::Init() {
	return false;
}

void LpIocpCore::Start() {
	if (!Bind(m_socket, SERVER_PORT))
		return;

	if (!Listen(m_socket))
		return;

	m_running = true;
}