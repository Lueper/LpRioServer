#include "pch.h"

#include "LpNetCore.h"

LpNetServer::LpNetServer() {
	m_iocpCore = new LpIocpCore();
	m_rioCore = new LpRioCore();
}

LpNetServer::~LpNetServer() {
	delete m_iocpCore;
	m_iocpCore = nullptr;
	delete m_rioCore;
	m_rioCore = nullptr;
}

void LpNetServer::Init(ENetMode mode) {
	if (!LpIocpCore::Startup())
		return;

	if (!InitRioCore()) {
		mode = ENetMode::IOCP;
		m_socket = LpIocpCore::CreateIocpSocket();
	}

	if (m_socket == INVALID_SOCKET)
		return;

	if (!LpIocpCore::LoadExFunction(m_socket, WSAID_ACCEPTEX, (LPVOID*)&LpIocpCore::AcceptEx)) {
		LpIocpCore::Close(m_socket);
		return;
	}

	if (!LpIocpCore::LoadExFunction(m_socket, WSAID_GETACCEPTEXSOCKADDRS, (LPVOID*)&LpIocpCore::GetAcceptExSockaddrs)) {
		LpIocpCore::Close(m_socket);
		return;
	}

	m_iocpCore->m_iocp = LpIocpCore::CreateIocpHandle();
	if (m_iocpCore->m_iocp == NULL)
		return;

	if (!LpIocpCore::RegisterIocpHandle(m_socket, m_iocpCore->m_iocp, CK_ACCEPT))
		return;

	LpIocpCore::SetReuseAddr(m_socket, true);

	LpIocpCore::Bind(m_socket, SERVER_PORT);
	LpIocpCore::Listen(m_socket);

	// @TODO: Worker Thread 쪽으로 이동
	//m_rioCore->m_rio.RIONotify(m_rioCore->m_rioCQ);
}

void LpNetServer::Start() {
	m_running = true;

	while (true) {

	}
}

void LpNetServer::Stop() {
	m_running = false;
}

void LpNetServer::Release() {

}

bool LpNetServer::InitRioCore() {
	m_socket = LpRioCore::CreateRioSocket();
	if (m_socket == INVALID_SOCKET)
		return false;

	if (!LpRioCore::LoadExFunctionTable(m_socket, WSAID_MULTIPLE_RIO, m_rioCore->m_rio))
		return false;

	DWORD poolSize = MAX_CONNECTIONS_COUNT * BUFFER_SIZE;
	m_rioCore->m_recvPool = (char*)VirtualAllocEx(GetCurrentProcess(), NULL, poolSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	m_rioCore->m_sendPool = (char*)VirtualAllocEx(GetCurrentProcess(), NULL, poolSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (m_rioCore->m_recvPool == nullptr || m_rioCore->m_sendPool == nullptr)
		return false;

	m_rioCore->m_recvBufId = LpRioCore::RegisterRioBuffer(m_rioCore->m_rio, m_rioCore->m_recvPool, poolSize);
	m_rioCore->m_sendBufId = LpRioCore::RegisterRioBuffer(m_rioCore->m_rio, m_rioCore->m_sendPool, poolSize);
	if (m_rioCore->m_recvBufId == RIO_INVALID_BUFFERID || m_rioCore->m_sendBufId == RIO_INVALID_BUFFERID)
		return false;

	m_rioCore->m_rioNotify.Type = RIO_IOCP_COMPLETION;
	m_rioCore->m_rioNotify.Iocp.IocpHandle = m_iocpCore->m_iocp;
	m_rioCore->m_rioNotify.Iocp.CompletionKey = (PVOID)CK_RIO;
	m_rioCore->m_rioNotify.Iocp.Overlapped = &m_iocpCore->m_overlapped;
	if (!LpRioCore::CreateRioCompletionQueue(m_rioCore->m_rio, CQ_SIZE, &m_rioCore->m_rioNotify))
		return false;
}