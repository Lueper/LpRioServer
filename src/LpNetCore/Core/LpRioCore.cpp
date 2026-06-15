#include "pch.h"
#include "LpNetCore.h"

#include "LpRioCore.h"
#include "LpIOContext.h"

LpRioCore::LpRioCore() {
	Startup();
	std::cout << "Rio 생성" << "\n";
}

LpRioCore::~LpRioCore() {
	if (m_recvBufferId != RIO_INVALID_BUFFERID)
		DeregisterRioBuffer(m_rio, m_recvBufferId);

	if (m_sendBufferId != RIO_INVALID_BUFFERID)
		DeregisterRioBuffer(m_rio, m_sendBufferId);

	if (m_recvPool != nullptr)
		VirtualFree(m_recvPool, 0, MEM_RELEASE);

	if (m_sendPool != nullptr)
		VirtualFree(m_sendPool, 0, MEM_RELEASE);

	CloseHandle(m_iocp);
	Close(m_socket);
	Cleanup();
	std::cout << "Rio 소멸" << "\n";
}

bool LpRioCore::Init() {
	m_socket = CreateRioSocket();
	if (m_socket == INVALID_SOCKET)
		return false;

	if (!LoadExFunction(m_socket, WSAID_ACCEPTEX, (LPVOID*)&AcceptEx))
		return false;

	if (!LoadExFunction(m_socket, WSAID_GETACCEPTEXSOCKADDRS, (LPVOID*)&GetAcceptExSockaddrs))
		return false;

	if (!LoadExFunctionTable(m_socket, WSAID_MULTIPLE_RIO, m_rio))
		return false;

	m_iocp = CreateHandle();
	if (m_iocp == NULL)
		return false;

	if (!RegisterSocket(m_socket, m_iocp, CK_ACCEPT))
		return false;

	m_recvPool = (char*)VirtualAlloc(NULL, BUFFER_POOL_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	m_sendPool = (char*)VirtualAlloc(NULL, BUFFER_POOL_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (m_recvPool == nullptr || m_sendPool == nullptr)
		return false;

	m_recvBufferId = RegisterRioBuffer(m_rio, m_recvPool, BUFFER_POOL_SIZE);
	m_sendBufferId = RegisterRioBuffer(m_rio, m_sendPool, BUFFER_POOL_SIZE);
	if (m_recvBufferId == RIO_INVALID_BUFFERID || m_sendBufferId == RIO_INVALID_BUFFERID)
		return false;

	m_rioNotify.Type = RIO_IOCP_COMPLETION;
	m_rioNotify.Iocp.IocpHandle = m_iocp;
	m_rioNotify.Iocp.CompletionKey = (PVOID)CK_RIO;
	m_rioNotify.Iocp.Overlapped = &m_overlapped;
	m_rioCQ = CreateRioCompletionQueue(m_rio, CQ_SIZE, &m_rioNotify);
	if (m_rioCQ == RIO_INVALID_CQ)
		return false;

	if (!SetReuseAddr(m_socket, true))
		return false;

	return true;
}

void LpRioCore::StartListen(uint16_t port, int threadCount) {
	m_threadCount = threadCount;

	if (!Bind(m_socket, port))
		return;

	if (!Listen(m_socket))
		return;

	m_rio.RIONotify(m_rioCQ);

	m_running = true;
}

void LpRioCore::StartConnect(int threadCount) {
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
			Close(m_socket);
			delete cctx;
			return;
		}
	}

	m_running = true;
}

void LpRioCore::Run() {
	while (m_running) {
		DWORD bytesTransferred = 0;
		ULONG_PTR completionKey = 0;
		OVERLAPPED* overlapped = nullptr;

		BOOL success = PopIocpContext(m_iocp, bytesTransferred, completionKey, overlapped, INFINITE);

		if (completionKey == CK_SHUTDOWN)
			break;

		if (overlapped == nullptr) {
			int error = GetLastError();
			LOG_ERROR("overlapped is null: ", error);
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
			case CK_RIO:
				OnRioCompletion();
				break;
		}
	}
}

void LpRioCore::RunClient() {
	while (m_running) {
		std::string message;
		std::getline(std::cin, message);
		if (message == "exit")
			break;

		ConnectContext* cctx = new ConnectContext();
		cctx->ioType = EIOType::Send;

		memcpy(&cctx->wsaBuf, message.c_str(), message.length());
		cctx->wsaBuf.len = static_cast<ULONG>(message.length());

		WSASend(
			m_socket,
			&cctx->wsaBuf,
			1, NULL, 0,
			&cctx->overlapped,
			NULL);
	}
}

void LpRioCore::Stop() {
	if (m_running == false)
		return;

	m_running = false;

	for (int i = 0; i < m_threadCount; i++) {
		PushIocpContext(m_iocp, 0, CK_SHUTDOWN, nullptr);
	}
}

void LpRioCore::PostAccept() {
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

void LpRioCore::OnAccept(AcceptContext* actx) {
	SetUpdateAcceptSocket(actx->acceptSock, m_socket);

	SOCKADDR* localAddr = nullptr;
	SOCKADDR* remoteAddr = nullptr;
	int localLength = 0;
	int remoteLength = 0;
	GetAcceptExSockaddrs(actx->addrBuf, 0, ADDR_LEN, ADDR_LEN, &localAddr, &localLength, &remoteAddr, &remoteLength);

	std::string ipAddress = GetIpAddress((SOCKADDR_IN&)*remoteAddr);

	ConnectionContext* cctx = new ConnectionContext();
	cctx->sock = actx->acceptSock;
	cctx->recvBuf.BufferId = m_recvBufferId;
	cctx->recvBuf.Offset = 0;  // index * BUFFER_SIZE;
	cctx->recvBuf.Length = BUFFER_SIZE;

	cctx->sendBuf.BufferId = m_sendBufferId;
	cctx->sendBuf.Offset = 0;  // index * BUFFER_SIZE;
	cctx->sendBuf.Length = BUFFER_SIZE;
	cctx->rq = m_rio.RIOCreateRequestQueue(cctx->sock, MAX_PENDING_RECVS, 1, MAX_PENDING_SENDS, 1, m_rioCQ, m_rioCQ, (PVOID)(ULONG_PTR)cctx);
	if (cctx->rq == RIO_INVALID_RQ) {
		Close(cctx->sock);
	}

	delete actx;
	PostAccept();
}

void LpRioCore::OnRioCompletion() {
	RIORESULT results[RIO_RESULTS_SIZE];

	ULONG count = m_rio.RIODequeueCompletion(m_rioCQ, results, RIO_RESULTS_SIZE);

	if (count == 0)
		return;

	if (count == RIO_CORRUPT_CQ) {
		// Shutdown();
		return;
	}

	for (ULONG i = 0; i < count; i++) {
		ConnectionContext* cctx = (ConnectionContext*)(ULONG_PTR)results[i].SocketContext;
		if (cctx == nullptr) {
			DWORD error = GetLastError();
			LOG_ERROR("ConnectionContext is null: %lu", error);
			continue;
		}

		if (results[i].Status != 0) {
			LOG_ERROR("RIOResult error: %ld", results[i].Status);
			Close(cctx->sock);
			delete cctx;
			continue;
		}

		EIOType ioType = (EIOType)results[i].RequestContext;
		switch (ioType) {
			case EIOType::Recv:
				ProcessRecv(results[i]);

				if (m_rio.RIOReceive(cctx->rq, &cctx->recvBuf, 1, 0, (PVOID)EIOType::Recv) == false) {
					DWORD error = GetLastError();
					LOG_ERROR("RIOReceive error: %lu", error);
					Close(cctx->sock);
					delete cctx;
				}
				break;
			case EIOType::Send:
				ProcessSend(results[i]);
				break;
		}
	}

	m_rio.RIONotify(m_rioCQ);
}

void LpRioCore::ProcessRecv(RIORESULT result) {
	LOG_DEBUG("Recv: %u bytes", result.BytesTransferred);
}

void LpRioCore::ProcessSend(RIORESULT result) {
	LOG_DEBUG("Send: %u bytes", result.BytesTransferred);
}