#pragma once

class LpSocketCore {
public:
	LpSocketCore() = default;
	virtual ~LpSocketCore() = default;

	virtual bool Init() = 0;
	virtual void Start() = 0;
	virtual void Run(int threadCount) = 0;

	virtual ENetMode GetMode() { return ENetMode::None; };

protected:
	bool Startup();
	bool Cleanup();

	SOCKET CreateIocpSocket();
	HANDLE CreateIocpHandle();

	bool RegisterIocpHandle(SOCKET socket, HANDLE iocp, ULONG_PTR completionKey = 0);
	bool LoadExFunction(SOCKET socket, GUID guid, LPVOID* outFunc);

	bool SetSockOpt(SOCKET socket, int level, int optName, const LPVOID optVal, int optLen);
	bool SetReuseAddr(SOCKET socket, BOOL optVal);
	bool SetNodelay(SOCKET socket, BOOL optVal);
	bool SetLinger(SOCKET socket, BOOL optVal, int time);
	bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);
	bool SetRecvBufSize(SOCKET socket, int size);
	bool SetSendBufSize(SOCKET socket, int size);

	bool Bind(SOCKET socket, unsigned short port);
	bool Listen(SOCKET socket);
	bool Close(SOCKET socket);
	bool CloseHandle(HANDLE handle);

	bool PopIocpEvent(HANDLE iocp, DWORD bytes, ULONG_PTR completionKey, OVERLAPPED* overlapped, DWORD timeoutMs = INFINITE);
	bool PushIocpEvent(HANDLE iocp, DWORD bytes = 0, ULONG_PTR completionKey = 0, OVERLAPPED* overlapped = nullptr);

	int GetLastError();
	std::string GetIpAddress(SOCKADDR_IN addr);

	// RIO
	SOCKET CreateRioSocket();
	RIO_CQ CreateRioCompletionQueue(RIO_EXTENSION_FUNCTION_TABLE& rio, DWORD cqSize, RIO_NOTIFICATION_COMPLETION* notification);
	RIO_RQ CreateRioRequestQueue(RIO_EXTENSION_FUNCTION_TABLE& rio, SOCKET socket, DWORD maxPendingRecv, DWORD maxRecvBuffers, DWORD maxPendingSend, DWORD maxSendBuffers, RIO_CQ recvCQ, RIO_CQ sendCQ, PVOID context);

	RIO_BUFFERID RegisterRioBuffer(RIO_EXTENSION_FUNCTION_TABLE& rio, char* buffer, DWORD size);
	void DeregisterRioBuffer(RIO_EXTENSION_FUNCTION_TABLE& rio, RIO_BUFFERID bufferId);
	bool LoadExFunctionTable(SOCKET socket, GUID guid, RIO_EXTENSION_FUNCTION_TABLE& rio);

	ULONG PopRioEvent(RIO_EXTENSION_FUNCTION_TABLE& rio, RIO_CQ cq, RIORESULT* results, ULONG size);
	bool NotifyRio(RIO_EXTENSION_FUNCTION_TABLE& rio, RIO_CQ cq);

protected:
	LPFN_ACCEPTEX				AcceptEx = nullptr;
	LPFN_GETACCEPTEXSOCKADDRS	GetAcceptExSockaddrs = nullptr;
	LPFN_CONNECTEX				ConnectEx = nullptr;
	LPFN_DISCONNECTEX			DisconnectEx = nullptr;

private:
};