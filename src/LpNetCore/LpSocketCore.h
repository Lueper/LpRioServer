#pragma once

#define SOCKET_SUCCESS 0

class LpSocketCore {
public:
	static SOCKET CreateIocpSocket();
	static SOCKET CreateRioSocket();
	static bool Startup();
	static bool Cleanup();
	static bool Close(SOCKET& socket);
	static bool Bind(SOCKET socket, unsigned short port);
	static bool Listen(SOCKET socket);

	// Set SockOpt
	static bool SetSockOpt(SOCKET socket, int level, int optName, const LPVOID optVal, int optLen);
	static bool SetReuseAddr(SOCKET socket, BOOL optVal);
	static bool SetNodelay(SOCKET socket, BOOL optVal);
	static bool SetLinger(SOCKET socket, BOOL optVal, int time);
	static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);
	static bool SetRecvBufSize(SOCKET socket, int size);
	static bool SetSendBufSize(SOCKET socket, int size);

	static bool LoadExFunction(SOCKET socket, GUID guid, LPVOID* outFunc);

	// RIO
	static bool LoadExFunctionTable(SOCKET socket, GUID guid, RIO_EXTENSION_FUNCTION_TABLE& outTable);
private:
};