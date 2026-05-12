#include "pch.h"

#include "LpNetCore.h"

LpRioCore::LpRioCore() {
	Startup();
	std::cout << "Rio 생성" << "\n";
}

LpRioCore::~LpRioCore() {
	if (m_recvBufferId != RIO_INVALID_BUFFERID)
		DeregisterRioBuffer(m_rio, m_recvBufferId);

	if (m_sendBufferId != RIO_INVALID_BUFFERID)
		DeregisterRioBuffer(m_rio, m_sendBufferId);

	if (m_recvPool != nullptr)
		VirtualFree(m_recvPool, 0, MEM_RELEASE);

	if (m_sendPool != nullptr)
		VirtualFree(m_sendPool, 0, MEM_RELEASE);

	CloseHandle(m_iocp);
	Close(m_socket);
	Cleanup();
	std::cout << "Rio 소멸" << "\n";
}

bool LpRioCore::Init() {
	m_socket = CreateRioSocket();
	if (m_socket == INVALID_SOCKET)
		return false;

	if (!LoadExFunction(m_socket, WSAID_ACCEPTEX, (LPVOID*)&AcceptEx))
		return false;

	if (!LoadExFunction(m_socket, WSAID_GETACCEPTEXSOCKADDRS, (LPVOID*)&GetAcceptExSockaddrs))
		return false;

	if (!LoadExFunctionTable(m_socket, WSAID_MULTIPLE_RIO, m_rio))
		return false;

	m_iocp = CreateIocpHandle();
	if (m_iocp == NULL)
		return false;

	if (!RegisterIocpHandle(m_socket, m_iocp, CK_ACCEPT))
		return false;

	m_recvPool = (char*)VirtualAlloc(NULL, BUFFER_POOL_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	m_sendPool = (char*)VirtualAlloc(NULL, BUFFER_POOL_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (m_recvPool == nullptr || m_sendPool == nullptr)
		return false;

	m_recvBufferId = RegisterRioBuffer(m_rio, m_recvPool, BUFFER_POOL_SIZE);
	m_sendBufferId = RegisterRioBuffer(m_rio, m_sendPool, BUFFER_POOL_SIZE);
	if (m_recvBufferId == RIO_INVALID_BUFFERID || m_sendBufferId == RIO_INVALID_BUFFERID)
		return false;

	m_rioNotify.Type = RIO_IOCP_COMPLETION;
	m_rioNotify.Iocp.IocpHandle = m_iocp;
	m_rioNotify.Iocp.CompletionKey = (PVOID)CK_RIO;
	m_rioNotify.Iocp.Overlapped = &m_overlapped;
	m_rioCQ = CreateRioCompletionQueue(m_rio, CQ_SIZE, &m_rioNotify);
	if (m_rioCQ == RIO_INVALID_CQ)
		return false;

	if (!SetReuseAddr(m_socket, true))
		return false;

	return true;
}

void LpRioCore::Start() {
	if (!Bind(m_socket, SERVER_PORT))
		return;

	if (!Listen(m_socket))
		return;

	m_rio.RIONotify(m_rioCQ);

	m_running = true;
}