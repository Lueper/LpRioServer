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
	return false;
}

void LpRioCore::Start() {

}