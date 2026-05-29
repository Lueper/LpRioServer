#include "pch.h"

#include "LpNetCore.h"

LpIocpCore::LpIocpCore() {
	Startup();
	std::cout << "Iocp 생성" << "\n";
}

LpIocpCore::~LpIocpCore() {
	CloseHandle(m_iocp);
	Close(m_socket);
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

void LpIocpCore::Start(int threadCount) {
	m_threadCount = threadCount;

	if (!Bind(m_socket, SERVER_PORT))
		return;

	if (!Listen(m_socket))
		return;

	for (ULONG i = 0; i < ACCEPT_POOL_SIZE; i++) {
		PostAccept();
	}

	m_running = true;
}

void LpIocpCore::Run() {
	while (m_running) {
		DWORD bytesTransferred = 0;
		ULONG_PTR completionKey = 0;
		OVERLAPPED* overlapped = nullptr;

		BOOL success = PopIocpEvent(m_iocp, bytesTransferred, completionKey, overlapped, INFINITE);

		if (completionKey == CK_SHUTDOWN)
			break;

		if (overlapped == nullptr) {
			int error = GetLastError();
			std::cout << "overlapped is null: " << error << "\n";
			continue;
		}

		auto actx = (AcceptContext*)overlapped;

		if (success == FALSE) {
			if (completionKey == CK_ACCEPT) {
				Close(actx->acceptSock);
				delete actx;

				PostAccept();
			}
			continue;
		}

		switch (completionKey) {
			case CK_ACCEPT:
				OnAccept(actx);
				break;
		}
	}
}

void LpIocpCore::Stop() {
}

void LpIocpCore::PostAccept() {
	AcceptContext* actx = new AcceptContext();

	actx->acceptSock = CreateIocpSocket();
	if (actx->acceptSock == INVALID_SOCKET) {
		delete actx;
		return;
	}

	DWORD bytes = 0;
	if (!AcceptEx(m_socket, actx->acceptSock, actx->addrBuf, 0, ADDR_LEN, ADDR_LEN, &bytes, &actx->overlapped)) {
		if (GetLastError() != ERROR_IO_PENDING) {
			Close(actx->acceptSock);
			delete actx;
			return;
		}
	}
}

void LpIocpCore::OnAccept(AcceptContext* actx) {
	setsockopt(actx->acceptSock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_socket, sizeof(m_socket));

	SOCKADDR* localAddr = nullptr;
	SOCKADDR* remoteAddr = nullptr;
	int localLength = 0;
	int remoteLength = 0;
	GetAcceptExSockaddrs(actx->addrBuf, 0, ADDR_LEN, ADDR_LEN, &localAddr, &localLength, &remoteAddr, &remoteLength);

	SOCKADDR_IN* remote = (SOCKADDR_IN*)remoteAddr;
	char remoteIp[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &remote->sin_addr, remoteIp, sizeof(remoteIp));

	delete actx;
	PostAccept();
}

void LpIocpCore::ProcessRecv(RIORESULT result) {
	LOG_DEBUG("Recv: %u bytes", result.BytesTransferred);
}

void LpIocpCore::ProcessSend(RIORESULT result) {
	LOG_DEBUG("Send: %u bytes", result.BytesTransferred);
}