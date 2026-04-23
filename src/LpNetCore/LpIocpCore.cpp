#include "pch.h"

#include "LpNetCore.h"

LPFN_ACCEPTEX				LpIocpCore::AcceptEx = nullptr;
LPFN_GETACCEPTEXSOCKADDRS	LpIocpCore::GetAcceptExSockaddrs = nullptr;
LPFN_CONNECTEX				LpIocpCore::ConnectEx = nullptr;
LPFN_DISCONNECTEX			LpIocpCore::DisconnectEx = nullptr;

LpIocpCore::LpIocpCore() {

}

LpIocpCore::~LpIocpCore() {

}

bool LpIocpCore::Startup() {
	WSADATA wsaData;
	return 0 == ::WSAStartup(MAKEWORD(2, 2), &wsaData);
}

bool LpIocpCore::Cleanup() {
	return SOCKET_ERROR != ::WSACleanup();
}

SOCKET LpIocpCore::CreateIocpSocket() {
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

HANDLE LpIocpCore::CreateIocpHandle() {
	return ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
}

bool LpIocpCore::RegisterIocpHandle(SOCKET socket, HANDLE iocp, ULONG_PTR completionKey) {
	return NULL != ::CreateIoCompletionPort((HANDLE)socket, iocp, completionKey, 0);
}

bool LpIocpCore::LoadExFunction(SOCKET socket, GUID guid, LPVOID* func) {
	DWORD bytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), func, sizeof(*func), &bytes, NULL, NULL);
}

bool LpIocpCore::SetSockOpt(SOCKET socket, int level, int optName, const LPVOID optVal, int optLen) {
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, (const char*)optVal, optLen);
}

bool LpIocpCore::SetReuseAddr(SOCKET socket, BOOL optVal) {
	return SetSockOpt(socket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
}

bool LpIocpCore::SetNodelay(SOCKET socket, BOOL optVal) {
	return SetSockOpt(socket, IPPROTO_TCP, TCP_NODELAY, &optVal, sizeof(optVal));
}

bool LpIocpCore::SetLinger(SOCKET socket, BOOL optVal, int time) {
	linger linger;
	linger.l_onoff = optVal;
	linger.l_linger = time;

	return SetSockOpt(socket, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
}

bool LpIocpCore::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket) {
	return SetSockOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, &listenSocket, sizeof(listenSocket));
}

bool LpIocpCore::SetRecvBufSize(SOCKET socket, int size) {
	return SetSockOpt(socket, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
}

bool LpIocpCore::SetSendBufSize(SOCKET socket, int size) {
	return SetSockOpt(socket, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
}

bool LpIocpCore::Bind(SOCKET socket, unsigned short port) {
	SOCKADDR_IN addr = {};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	addr.sin_port = ::htons(port);

	return SOCKET_ERROR != ::bind(socket, (const SOCKADDR*)&addr, sizeof(addr));
}

bool LpIocpCore::Listen(SOCKET socket) {
	return SOCKET_ERROR != ::listen(socket, SOMAXCONN);
}

bool LpIocpCore::Close(SOCKET& socket) {
	if (socket == INVALID_SOCKET)
		return false;

	::closesocket(socket);
	socket = INVALID_SOCKET;

	return true;
}

bool LpIocpCore::PopIocpEvent(HANDLE iocp, DWORD bytes, ULONG_PTR completionKey, OVERLAPPED* overlapped, DWORD timeoutMs) {
	return ::GetQueuedCompletionStatus(iocp, &bytes, &completionKey, &overlapped, timeoutMs);
}

bool LpIocpCore::PushIocpEvent(HANDLE iocp, DWORD bytes, ULONG_PTR completionKey, OVERLAPPED* overlapped) {
	return ::PostQueuedCompletionStatus(iocp, bytes, completionKey, overlapped);
}

std::string LpIocpCore::GetIpAddress(SOCKADDR_IN addr) {
	char ipAddress[INET_ADDRSTRLEN] = { 0, };
	::inet_ntop(AF_INET, &addr.sin_addr, ipAddress, sizeof(ipAddress));

	return std::string(ipAddress);
}