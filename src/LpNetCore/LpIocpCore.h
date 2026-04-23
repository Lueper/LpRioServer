#pragma once

class LpIocpCore {
public:
	LpIocpCore();
	~LpIocpCore();

	static bool Startup();
	static bool Cleanup();

	static SOCKET CreateIocpSocket();
	static HANDLE CreateIocpHandle();

	static bool RegisterIocpHandle(SOCKET socket, HANDLE iocp, ULONG_PTR completionKey = 0);
	static bool LoadExFunction(SOCKET socket, GUID guid, LPVOID* func);

	static bool SetSockOpt(SOCKET socket, int level, int optName, const LPVOID optVal, int optLen);
	static bool SetReuseAddr(SOCKET socket, BOOL optVal);
	static bool SetNodelay(SOCKET socket, BOOL optVal);
	static bool SetLinger(SOCKET socket, BOOL optVal, int time);
	static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);
	static bool SetRecvBufSize(SOCKET socket, int size);
	static bool SetSendBufSize(SOCKET socket, int size);

	static bool Bind(SOCKET socket, unsigned short port);
	static bool Listen(SOCKET socket);
	static bool Close(SOCKET& socket);

	static bool PopIocpEvent(HANDLE iocp, DWORD bytes, ULONG_PTR completionKey, OVERLAPPED* overlapped, DWORD timeoutMs = INFINITE);
	static bool PushIocpEvent(HANDLE iocp, DWORD bytes = 0, ULONG_PTR completionKey = 0, OVERLAPPED* overlapped = nullptr);

	static std::string GetIpAddress(SOCKADDR_IN addr);

	static LPFN_ACCEPTEX				AcceptEx;
	static LPFN_GETACCEPTEXSOCKADDRS	GetAcceptExSockaddrs;
	static LPFN_CONNECTEX				ConnectEx;
	static LPFN_DISCONNECTEX			DisconnectEx;

private:
	HANDLE m_iocp = nullptr;
	OVERLAPPED m_overlapped = {};
};