//#pragma once
//
//class LpSocketCore {
//public:
//	static bool Startup();
//	static bool Cleanup();
//
//	static SOCKET CreateIocpSocket();
//	static HANDLE CreateIocpHandle();
//
//	static bool LoadExFunction(SOCKET socket, GUID guid, LPVOID* outFunc);
//	static bool RegisterIocpHandle(SOCKET socket, HANDLE iocp, ULONG_PTR completionKey = 0);
//
//	static bool SetSockOpt(SOCKET socket, int level, int optName, const LPVOID optVal, int optLen);
//	static bool SetReuseAddr(SOCKET socket, BOOL optVal);
//	static bool SetNodelay(SOCKET socket, BOOL optVal);
//	static bool SetLinger(SOCKET socket, BOOL optVal, int time);
//	static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);
//	static bool SetRecvBufSize(SOCKET socket, int size);
//	static bool SetSendBufSize(SOCKET socket, int size);
//
//	static bool Bind(SOCKET socket, unsigned short port);
//	static bool Listen(SOCKET socket);
//	static bool Close(SOCKET& socket);
//
//	static bool PopIocpEvent(HANDLE iocp, DWORD bytes, ULONG_PTR completionKey, OVERLAPPED* overlapped, DWORD timeoutMs = INFINITE);
//	static bool PushIocpEvent(HANDLE iocp, DWORD bytes = 0, ULONG_PTR completionKey = 0, OVERLAPPED* overlapped = nullptr);
//
//	static std::string GetIpAddress(SOCKADDR_IN addr);
//
//	// RIO
//	static SOCKET CreateRioSocket();
//	static RIO_CQ CreateRioCompletionQueue(RIO_EXTENSION_FUNCTION_TABLE& rio, DWORD cqSize, RIO_NOTIFICATION_COMPLETION* notification);
//	static RIO_RQ CreateRioRequestQueue(RIO_EXTENSION_FUNCTION_TABLE& rio, SOCKET socket, DWORD maxPendingRecv, DWORD maxRecvBuffers, DWORD maxPendingSend, DWORD maxSendBuffers, RIO_CQ recvCQ, RIO_CQ sendCQ, PVOID context);
//
//	static bool LoadExFunctionTable(SOCKET socket, GUID guid, RIO_EXTENSION_FUNCTION_TABLE& rio);
//	static RIO_BUFFERID RegisterRioBuffer(RIO_EXTENSION_FUNCTION_TABLE& rio, char* buffer, DWORD size);
//
//	static ULONG PopRioEvent(RIO_EXTENSION_FUNCTION_TABLE& rio, RIO_CQ cq, RIORESULT* results, ULONG size);
//	static bool NotifyRio(RIO_EXTENSION_FUNCTION_TABLE& rio, RIO_CQ cq);
//private:
//};