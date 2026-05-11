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
	m_socket = CreateIocpSocket();
	if (m_socket == INVALID_SOCKET)
		return false;

	if (!LoadExFunction(m_socket, WSAID_ACCEPTEX, (LPVOID*)&AcceptEx))
		return false;

	if (!LoadExFunction(m_socket, WSAID_GETACCEPTEXSOCKADDRS, (LPVOID*)&GetAcceptExSockaddrs))
		return false;

	m_iocp = CreateIocpHandle();
	if (m_iocp == NULL)
		return false;

	if (!RegisterIocpHandle(m_socket, m_iocp, CK_ACCEPT))
		return false;

	if (!SetReuseAddr(m_socket, true))
		return false;

	return true;
}

void LpIocpCore::Start() {
	if (!Bind(m_socket, SERVER_PORT))
		return;

	if (!Listen(m_socket))
		return;

	m_running = true;
}