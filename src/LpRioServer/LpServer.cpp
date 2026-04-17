#include "LpServer.h"

LpServer::LpServer() {
	m_socket = INVALID_SOCKET;
}

LpServer::~LpServer() {

}

bool LpServer::Init() {
	// 1) Startup
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return false;
	}

	// 2) Bind Function
	m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED || WSA_FLAG_REGISTERED_IO);
	if (m_socket == INVALID_SOCKET) {
		return false;
	}

	DWORD bytes;
	GUID acceptId = WSAID_ACCEPTEX;
	int result = WSAIoctl(m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
						  &acceptId, sizeof(acceptId), &m_lpfnAcceptEx, sizeof(m_lpfnAcceptEx), &bytes, NULL, NULL);
	if (result != 0) {
		closesocket(m_socket);
		return false;
	}

	GUID sockaddrsId = WSAID_GETACCEPTEXSOCKADDRS;
	result = WSAIoctl(m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
					  &sockaddrsId, sizeof(sockaddrsId), &m_lpfnGetAcceptExSockaddrs, sizeof(m_lpfnGetAcceptExSockaddrs), &bytes, NULL, NULL);
	if (result != 0) {
		closesocket(m_socket);
		return false;
	}

	m_rio.cbSize = sizeof(RIO_EXTENSION_FUNCTION_TABLE);
	GUID rioId = WSAID_MULTIPLE_RIO;
	result = WSAIoctl(m_socket, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER,
					  &rioId, sizeof(rioId), &m_rio, sizeof(m_rio), &bytes, NULL, NULL);
	if (result != 0) {
		closesocket(m_socket);
		return false;
	}

	// 3) Register Buffer
	size_t poolSize = static_cast<size_t>(MAX_CONNECTIONS_COUNT * BUFFER_SIZE);
	m_recvPool = reinterpret_cast<char*>(VirtualAllocEx(GetCurrentProcess(), NULL, poolSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	m_sendPool = reinterpret_cast<char*>(VirtualAllocEx(GetCurrentProcess(), NULL, poolSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	if (m_recvPool == nullptr || m_sendPool == nullptr)
		return false;

	m_recvBufId = m_rio.RIORegisterBuffer(m_recvPool, static_cast<DWORD>(poolSize));
	m_sendBufId = m_rio.RIORegisterBuffer(m_sendPool, static_cast<DWORD>(poolSize));
	if (m_recvBufId == RIO_INVALID_BUFFERID || m_sendBufId == RIO_INVALID_BUFFERID)
		return false;

	m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_iocp == NULL)
		return false;

	RIO_NOTIFICATION_COMPLETION rioNotify = {};
	rioNotify.Type = RIO_IOCP_COMPLETION;
	rioNotify.Iocp.IocpHandle = m_iocp;
	rioNotify.Iocp.CompletionKey = (PVOID)CK_RIO;
	rioNotify.Iocp.Overlapped = &m_overlapped;
	m_rioCQ = m_rio.RIOCreateCompletionQueue(CQ_SIZE, &rioNotify);
	if (m_rioCQ == RIO_INVALID_CQ)
		return false;

	BOOL reuseAddr = TRUE;
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&reuseAddr), sizeof(reuseAddr));

	sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(SERVER_PORT);
	if (bind(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
		return false;

	return true;
}

void LpServer::Start() {
	m_running = true;

		// 4) Listen
	if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
		return;

	if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_socket), m_iocp, CK_ACCEPT, 0) == nullptr)
		return;

	// Initial RIO notification
	m_rio.RIONotify(m_rioCQ);

	for (ULONG i = 0; i < ACCEPT_POOL_SIZE; i++) {
		PostAccept();
	}

	LOG_INFO("Server Start Success");

	Run();
}

void LpServer::Stop() {
	m_running = false;

	LOG_INFO("Server Stop");
}

void LpServer::Release() {
	LOG_INFO("Server Release");
}

void LpServer::Run() {
	int threadCount = std::thread::hardware_concurrency();

	for (int i = 0; i < threadCount; i++) {
		std::thread* thread = new std::thread([this] {
			WorkerThread();
		});
		m_ioThreadVec.push_back(thread);
	}

	for (auto* thread : m_ioThreadVec) {
		if (thread->joinable())
			thread->join();
	}
	m_ioThreadVec.clear();
}

void LpServer::WorkerThread() {
	while (m_running) {
		DWORD bytesTransferred = 0;
		ULONG_PTR completionKey = 0;
		OVERLAPPED* overlapped = nullptr;

		BOOL success = GetQueuedCompletionStatus(m_iocp, &bytesTransferred, &completionKey, &overlapped, INFINITE);

		if (completionKey == CK_SHUTDOWN)
			break;

		if (overlapped == nullptr) {
			DWORD error = WSAGetLastError();
			LOG_ERROR("overlapped is null: %lu", error);
			continue;
		}

		auto actx = (AcceptContext*)overlapped;

		if (success == FALSE) {
			if (completionKey == CK_ACCEPT) {
				closesocket(actx->acceptSock);
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

bool LpServer::PostAccept() {
	AcceptContext* actx = new AcceptContext();

	actx->acceptSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (actx->acceptSock == INVALID_SOCKET) {
		delete actx;
		return false;
	}

	DWORD bytes = 0;
	BOOL result = m_lpfnAcceptEx(m_socket, actx->acceptSock, actx->addrBuf, 0, ADDR_LEN, ADDR_LEN, &bytes, &actx->overlapped);
	if (result != 0) {
		closesocket(actx->acceptSock);
		delete actx;
		return false;
	}

	return true;
}

void LpServer::OnAccept(AcceptContext* actx) {
	setsockopt(actx->acceptSock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&m_socket, sizeof(m_socket));
	
	SOCKADDR* localAddr = nullptr;
	SOCKADDR* remoteAddr = nullptr;
	int localLength = 0;
	int remoteLength = 0;
	m_lpfnGetAcceptExSockaddrs(actx->addrBuf, 0, ADDR_LEN, ADDR_LEN, &localAddr, &localLength, &remoteAddr, &remoteLength);

	SOCKADDR_IN* remote = (SOCKADDR_IN*)remoteAddr;
	char remoteIp[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &remote->sin_addr, remoteIp, sizeof(remoteIp));

	ConnectionContext* cctx = new ConnectionContext();
	cctx->sock = actx->acceptSock;
	cctx->recvBuf.BufferId = m_recvBufId;
	cctx->recvBuf.Offset = 0;  // index * BUFFER_SIZE;
	cctx->recvBuf.Length = BUFFER_SIZE;

	cctx->sendBuf.BufferId = m_sendBufId;
	cctx->sendBuf.Offset = 0;  // index * BUFFER_SIZE;
	cctx->sendBuf.Length = BUFFER_SIZE;
	cctx->rq = m_rio.RIOCreateRequestQueue(cctx->sock, MAX_PENDING_RECVS, 1, MAX_PENDING_SENDS, 1, m_rioCQ, m_rioCQ, (PVOID)(ULONG_PTR)cctx);
	if (cctx->rq == RIO_INVALID_RQ) {
		closesocket(cctx->sock);
	}

	delete actx;
	PostAccept();
}

void LpServer::OnRioCompletion() {
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

void LpServer::ProcessRecv(RIORESULT result) {
	LOG_DEBUG("Recv: %u bytes", result.BytesTransferred);
}

void LpServer::ProcessSend(RIORESULT result) {
	LOG_DEBUG("Send: %u bytes", result.BytesTransferred);
}