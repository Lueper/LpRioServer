#include "pch.h"
#include "LpNetCore.h"

#include "LpIocpCore.h"
#include "LpIOContext.h"

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
	m_socket = CreateSocket();
	if (m_socket == INVALID_SOCKET)
		return false;

	if (!LoadExFunction(m_socket, WSAID_ACCEPTEX, (LPVOID*)&AcceptEx))
		return false;

	if (!LoadExFunction(m_socket, WSAID_GETACCEPTEXSOCKADDRS, (LPVOID*)&GetAcceptExSockaddrs))
		return false;

	if (!LoadExFunction(m_socket, WSAID_CONNECTEX, (LPVOID*)&ConnectEx))
		return false;

	m_iocp = CreateHandle();
	if (m_iocp == NULL)
		return false;

		if (!RegisterSocket(m_socket, m_iocp, CK_ACCEPT))
			return false;

	if (!SetReuseAddr(m_socket, true))
		return false;

	return true;
}

void LpIocpCore::StartListen(uint16_t port, int threadCount) {
	m_threadCount = threadCount;

	if (!Bind(m_socket, port))
		return;

	if (!Listen(m_socket))
		return;

	for (ULONG i = 0; i < ACCEPT_POOL_SIZE; i++) {
		PostAccept();
	}

	m_running = true;
}

void LpIocpCore::StartConnect(int threadCount) {
	m_threadCount = threadCount;

	if (!Bind(m_socket, 0))
		return;

	SOCKADDR_IN addr = {};
	addr.sin_family = AF_INET;
	::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_port = ::htons(SERVER_PORT);

	ConnectContext* cctx = new ConnectContext();
	cctx->ioType = EIOType::Connect;

	if (!ConnectEx(m_socket, (SOCKADDR*)&addr, sizeof(addr), NULL, 0, NULL, &cctx->overlapped)) {
		if (GetLastError() != ERROR_IO_PENDING) {
			LOG_ERROR("Connect Failed: ", GetLastError());
			Close(m_socket);
			delete cctx;
			return;
		}
	}

	LOG_INFO("Connect Complete");

	m_running = true;
}

void LpIocpCore::Run() {
	while (m_running) {
		DWORD bytes = 0;
		ULONG_PTR completionKey = 0;
		OVERLAPPED* overlapped = nullptr;

		BOOL success = PopIocpContext(m_iocp, bytes, completionKey, overlapped, INFINITE);

		if (overlapped == nullptr) {
			LOG_ERROR("overlapped is null: ", GetLastError());
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
			case CK_IO:
				auto cctx = (ConnectContext*)overlapped;

				switch (cctx->ioType) {
					case EIOType::Recv:
						OnRecv(cctx, bytes);
						break;
					case EIOType::Send:
						OnSend(cctx, bytes);
						break;
				}
				break;
		}
	}
}

void LpIocpCore::RunClient() {
	while (true) {
		std::string message;
		std::getline(std::cin, message);
		if (message == "exit")
			break;

		DWORD bytes = 0;
		ConnectContext* cctx = new ConnectContext();
		cctx->ioType = EIOType::Send;
		cctx->wsaBuf.buf = cctx->buf;
		cctx->wsaBuf.len = BUFFER_SIZE;

		memcpy(&cctx->buf, message.c_str(), message.length());
		cctx->wsaBuf.len = static_cast<ULONG>(message.length());

		WSASend(
			m_socket,
			&cctx->wsaBuf,
			1, &bytes, 0,
			&cctx->overlapped,
			NULL);
	}
}

void LpIocpCore::Stop() {
	if (m_running == false)
		return;

	m_running = false;

	for (int i = 0; i < m_threadCount; i++) {
		PushIocpContext(m_iocp, 0, CK_SHUTDOWN, nullptr);
	}
}

void LpIocpCore::PostAccept() {
	AcceptContext* actx = new AcceptContext();

	actx->acceptSock = CreateSocket();
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
	SetUpdateAcceptSocket(actx->acceptSock, m_socket);

	SOCKADDR* localAddr = nullptr;
	SOCKADDR* remoteAddr = nullptr;
	int localLength = 0;
	int remoteLength = 0;
	GetAcceptExSockaddrs(actx->addrBuf, 0, ADDR_LEN, ADDR_LEN, &localAddr, &localLength, &remoteAddr, &remoteLength);

	std::string ipAddress = GetIpAddress((SOCKADDR_IN&)*remoteAddr);

	RegisterIocpHandle(actx->acceptSock, m_iocp, CK_IO);

	ConnectContext* cctx = new ConnectContext();
	cctx->sock = actx->acceptSock;
	cctx->ioType = EIoType::Recv;
	cctx->wsaBuf.buf = cctx->buf;
	cctx->wsaBuf.len = BUFFER_SIZE;

	DWORD recvLen = 0;
	DWORD flags = 0;
	if (::WSARecv(actx->acceptSock, &cctx->wsaBuf, 1, &recvLen, &flags, &cctx->overlapped, nullptr) == SOCKET_ERROR) {
		if (::WSAGetLastError() != ERROR_IO_PENDING) {
			LOG_ERROR("Recv Failed: ", ::WSAGetLastError());
			Close(actx->acceptSock);
			delete actx;
			return;
		}
	}

	LOG_INFO("Client Connected: ", actx->acceptSock);

	//delete actx;
	PostAccept();
}

void LpIocpCore::OnRecv(ConnectContext* cctx, DWORD bytesTransferred) {
	LOG_INFO("Client Recv: ", bytesTransferred);

		cctx->ioType = EIoType::Send;
		cctx->wsaBuf.len = bytesTransferred;
		DWORD sendLen = 0;
		if (::WSASend(cctx->sock, &cctx->wsaBuf, 1, &sendLen, 0, &cctx->overlapped, nullptr) == SOCKET_ERROR) {
			if (::WSAGetLastError() != ERROR_IO_PENDING) {
				LOG_ERROR("Send Failed: ", ::WSAGetLastError());
				Close(cctx->sock);
				delete cctx;
				return;
			}
		}
}

void LpIocpCore::OnSend(ConnectContext* cctx, DWORD bytes) {
	LOG_INFO("Client Send Complete: ", bytes, " bytes.");

	cctx->ioType = EIOType::Recv;
	cctx->wsaBuf.buf = cctx->buf;
	cctx->wsaBuf.len = BUFFER_SIZE;

	DWORD recvLen = 0;
	DWORD flags = 0;
	if (::WSARecv(m_socket, &cctx->wsaBuf, 1, &recvLen, &flags, &cctx->overlapped, nullptr) == SOCKET_ERROR) {
		if (::WSAGetLastError() != ERROR_IO_PENDING) {
			LOG_ERROR("Next Recv Failed: ", ::WSAGetLastError());
			Close(cctx->sock);
			delete cctx;
		}
	}
}

void LpIocpCore::ProcessRecv(RIORESULT result) {
	LOG_DEBUG("Recv: %u bytes", result.BytesTransferred);
}

void LpIocpCore::ProcessSend(RIORESULT result) {
	LOG_DEBUG("Send: %u bytes", result.BytesTransferred);
}