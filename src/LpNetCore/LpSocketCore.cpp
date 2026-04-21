#include "pch.h"

#include "LpSocketCore.h"

SOCKET LpSocketCore::CreateIocpSocket() {
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

SOCKET LpSocketCore::CreateRioSocket() {
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED | WSA_FLAG_REGISTERED_IO);
}

bool LpSocketCore::Startup() {
	WSADATA wsaData;
	return SOCKET_SUCCESS == ::WSAStartup(MAKEWORD(2, 2), &wsaData);
}

bool LpSocketCore::Cleanup() {
	return SOCKET_ERROR != ::WSACleanup();
}

bool LpSocketCore::Close(SOCKET& socket) {
	if (socket == INVALID_SOCKET)
		return false;

	::closesocket(socket);
	socket = INVALID_SOCKET;

	return true;
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

bool LpSocketCore::LoadExFunction(SOCKET socket, GUID guid, LPVOID* outFunc) {
	DWORD bytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), outFunc, sizeof(*outFunc), &bytes, NULL, NULL);
}

bool LpSocketCore::LoadExFunctionTable(SOCKET socket, GUID guid, RIO_EXTENSION_FUNCTION_TABLE& outTable) {
	DWORD bytes = 0;
	outTable.cbSize = sizeof(outTable);
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &outTable, sizeof(outTable), &bytes, NULL, NULL);
}