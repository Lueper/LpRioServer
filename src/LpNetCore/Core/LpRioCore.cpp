#include "pch.h"

#include "LpNetCore.h"

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

	m_iocp = CreateIocpHandle();
	if (m_iocp == NULL)
		return false;

	if (!RegisterIocpHandle(m_socket, m_iocp, CK_ACCEPT))
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

void LpRioCore::Start() {
	if (!Bind(m_socket, SERVER_PORT))
		return;

	if (!Listen(m_socket))
		return;

	m_rio.RIONotify(m_rioCQ);

	m_running = true;
}

void LpRioCore::PostAccept() {
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

void LpRioCore::OnAccept(AcceptContext* actx) {
	setsockopt(actx->acceptSock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_socket, sizeof(m_socket));

	SOCKADDR* localAddr = nullptr;
	SOCKADDR* remoteAddr = nullptr;
	int localLength = 0;
	int remoteLength = 0;
	GetAcceptExSockaddrs(actx->addrBuf, 0, ADDR_LEN, ADDR_LEN, &localAddr, &localLength, &remoteAddr, &remoteLength);

	SOCKADDR_IN* remote = (SOCKADDR_IN*)remoteAddr;
	char remoteIp[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &remote->sin_addr, remoteIp, sizeof(remoteIp));

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
		closesocket(cctx->sock);
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
			DWORD error = WSAGetLastError();
			LOG_ERROR("ConnectionContext is null: %lu", error);
			continue;
		}

		if (results[i].Status != 0) {
			LOG_ERROR("RIOResult error: %ld", results[i].Status);
			closesocket(cctx->sock);
			delete cctx;
			continue;
		}

		EIoType ioType = (EIoType)results[i].RequestContext;
		switch (ioType) {
			case EIoType::Recv:
				ProcessRecv(results[i]);

				if (m_rio.RIOReceive(cctx->rq, &cctx->recvBuf, 1, 0, (PVOID)EIoType::Recv) == false) {
					DWORD error = WSAGetLastError();
					LOG_ERROR("RIOReceive error: %lu", error);
					closesocket(cctx->sock);
					delete cctx;
				}
				break;
			case EIoType::Send:
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