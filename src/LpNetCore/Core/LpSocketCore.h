#pragma once

#include "LpGlobal.h"

class LpSocketCore {
public:
	LpSocketCore() = default;
	virtual ~LpSocketCore() = default;

	virtual bool Init() = 0;
	virtual void StartListen(uint16_t port, int threadCount) = 0;
	virtual void StartConnect(int threadCount) = 0;
	virtual void Run() = 0;
	virtual void RunClient() = 0;
	virtual void Stop() = 0;

	virtual ENetMode GetMode() { return ENetMode::None; };

protected:
	bool Startup();
	bool Cleanup();

	SOCKET CreateSocket();
	HANDLE CreateHandle();

	bool RegisterSocket(SOCKET socket, HANDLE iocp, ULONG_PTR completionKey = 0);
	bool LoadExFunction(SOCKET socket, GUID guid, LPVOID* outFunc);

	bool SetSockOpt(SOCKET socket, int level, int optName, const LPVOID optVal, int optLen);
	bool SetReuseAddr(SOCKET socket, BOOL optVal);
	bool SetNodelay(SOCKET socket, BOOL optVal);
	bool SetLinger(SOCKET socket, BOOL optVal, int time);
	bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);
	bool SetUpdateConnectSocket(SOCKET socket);
	bool SetRecvBufSize(SOCKET socket, int size);
	bool SetSendBufSize(SOCKET socket, int size);

	bool Bind(SOCKET socket, unsigned short port);
	bool Listen(SOCKET socket);
	bool Close(SOCKET socket);
	bool CloseHandle(HANDLE handle);

	bool Recv(SOCKET socket, WSABUF& wsaBuf, LPOVERLAPPED overlapped);
	bool Send(SOCKET socket, WSABUF& wsaBuf, LPOVERLAPPED overlapped);

	bool PopIocpContext(HANDLE iocp, DWORD& bytes, ULONG_PTR& completionKey, LPOVERLAPPED& overlapped, DWORD timeoutMs = INFINITE);
	bool PushIocpContext(HANDLE iocp, DWORD bytes = 0, ULONG_PTR completionKey = 0, OVERLAPPED* overlapped = nullptr);

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

	std::atomic<bool> m_running = false;
	int m_threadCount = 0;

private:
};