#include "pch.h"

#include "LpSocketCore.h"

bool LpSocketCore::Startup() {
	WSADATA wsaData;
	return 0 == ::WSAStartup(MAKEWORD(2, 2), &wsaData);
}

bool LpSocketCore::Cleanup() {
	return SOCKET_ERROR != ::WSACleanup();
}

SOCKET LpSocketCore::CreateIocpSocket() {
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

HANDLE LpSocketCore::CreateIocpHandle() {
	return ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
}

bool LpSocketCore::LoadExFunction(SOCKET socket, GUID guid, LPVOID* outFunc) {
	DWORD bytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), outFunc, sizeof(*outFunc), &bytes, NULL, NULL);
}

bool LpSocketCore::RegisterIocpHandle(SOCKET socket, HANDLE iocp, ULONG_PTR completionKey) {
	return NULL != ::CreateIoCompletionPort((HANDLE)socket, iocp, completionKey, 0);
}

bool LpSocketCore::SetSockOpt(SOCKET socket, int level, int optName, const LPVOID optVal, int optLen) {
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, (const char*)optVal, optLen);
}

bool LpSocketCore::SetReuseAddr(SOCKET socket, BOOL optVal) {
	return SetSockOpt(socket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
}

bool LpSocketCore::SetNodelay(SOCKET socket, BOOL optVal) {
	return SetSockOpt(socket, IPPROTO_TCP, TCP_NODELAY, &optVal, sizeof(optVal));
}

bool LpSocketCore::SetLinger(SOCKET socket, BOOL optVal, int time) {
	linger linger;
	linger.l_onoff = optVal;
	linger.l_linger = time;

	return SetSockOpt(socket, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
}

bool LpSocketCore::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket) {
	return SetSockOpt(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, &listenSocket, sizeof(listenSocket));
}

bool LpSocketCore::SetRecvBufSize(SOCKET socket, int size) {
	return SetSockOpt(socket, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
}

bool LpSocketCore::SetSendBufSize(SOCKET socket, int size) {
	return SetSockOpt(socket, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
}

bool LpSocketCore::Bind(SOCKET socket, unsigned short port) {
	SOCKADDR_IN addr = {};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	addr.sin_port = ::htons(port);

	return SOCKET_ERROR != ::bind(socket, (const SOCKADDR*)&addr, sizeof(addr));
}

bool LpSocketCore::Listen(SOCKET socket) {
	return SOCKET_ERROR != ::listen(socket, SOMAXCONN);
}

bool LpSocketCore::Close(SOCKET& socket) {
	if (socket == INVALID_SOCKET)
		return false;

	::closesocket(socket);
	socket = INVALID_SOCKET;

	return true;
}

bool LpSocketCore::PopIocpEvent(HANDLE iocp, DWORD bytes, ULONG_PTR completionKey, OVERLAPPED* overlapped, DWORD timeoutMs) {
	return ::GetQueuedCompletionStatus(iocp, &bytes, &completionKey, &overlapped, timeoutMs);
}

bool LpSocketCore::PushIocpEvent(HANDLE iocp, DWORD bytes, ULONG_PTR completionKey, OVERLAPPED* overlapped)
{
	return false;
}

std::string LpSocketCore::GetIpAddress(SOCKADDR_IN addr) {
	char ipAddress[INET_ADDRSTRLEN] = { 0, };
	::inet_ntop(AF_INET, &addr.sin_addr, ipAddress, sizeof(ipAddress));

	return std::string(ipAddress);
}

SOCKET LpSocketCore::CreateRioSocket() {
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED | WSA_FLAG_REGISTERED_IO);
}

RIO_CQ LpSocketCore::CreateRioCompletionQueue(RIO_EXTENSION_FUNCTION_TABLE& rio, DWORD cqSize, RIO_NOTIFICATION_COMPLETION* notification) {
	return rio.RIOCreateCompletionQueue(cqSize, notification);
}

RIO_RQ LpSocketCore::CreateRioRequestQueue(RIO_EXTENSION_FUNCTION_TABLE& rio, SOCKET socket, DWORD maxPendingRecv, DWORD maxRecvBuffers, DWORD maxPendingSend, DWORD maxSendBuffers, RIO_CQ recvCQ, RIO_CQ sendCQ, PVOID context) {
	return rio.RIOCreateRequestQueue(socket, maxPendingRecv, maxRecvBuffers, maxPendingSend, maxSendBuffers, recvCQ, sendCQ, context);
}

bool LpSocketCore::LoadExFunctionTable(SOCKET socket, GUID guid, RIO_EXTENSION_FUNCTION_TABLE& rio) {
	DWORD bytes = 0;
	rio.cbSize = sizeof(rio);
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &rio, sizeof(rio), &bytes, NULL, NULL);
}

RIO_BUFFERID LpSocketCore::RegisterRioBuffer(RIO_EXTENSION_FUNCTION_TABLE& rio, char* buffer, DWORD size) {
	return rio.RIORegisterBuffer(buffer, size);
}

ULONG LpSocketCore::PopRioEvent(RIO_EXTENSION_FUNCTION_TABLE& rio, RIO_CQ cq, RIORESULT* results, ULONG size) {
	return rio.RIODequeueCompletion(cq, results, size);
}

bool LpSocketCore::NotifyRio(RIO_EXTENSION_FUNCTION_TABLE& rio, RIO_CQ cq) {
	return 0 == rio.RIONotify(cq);
}