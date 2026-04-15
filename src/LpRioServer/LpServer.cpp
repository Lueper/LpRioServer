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
	m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
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
	if (m_iocp == nullptr)
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

	// 4) Listen
	if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
		return false;

	if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_socket), m_iocp, CK_ACCEPT, 0) == nullptr)
		return false;

	// Initial RIO notification
	m_rio.RIONotify(m_rioCQ);

	return true;
}

void LpServer::Start() {
	m_running = true;

	LOG_INFO("Server Start");
}

void LpServer::Stop() {
	m_running = false;

	LOG_INFO("Server Stop");
}

void LpServer::Release() {
	LOG_INFO("Server Release");
}

void LpServer::Run() {

}