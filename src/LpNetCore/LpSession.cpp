#include "pch.h"
#include "LpNetCore.h"

#include "LpSession.h"
#include "Utility/LpBuffer.h"
#include "LpIOContext.h"

LpSession::LpSession() {

}

LpSession::LpSession(SOCKET socket) {
	m_socket = socket;
	m_recvBuffer = new LpBuffer(BUFFER_SIZE);
	m_sendBuffer = new LpBuffer(BUFFER_SIZE);
}

LpSession::~LpSession() {
	Disconnect();

	m_recvBuffer->Clear();
	m_sendBuffer->Clear();
	delete m_recvBuffer;
	delete m_sendBuffer;
}

void LpSession::Disconnect() {
	if (m_connected == false)
		return;

	m_connected = false;

	if (m_socket == INVALID_SOCKET)
		return;

	::shutdown(m_socket, SD_BOTH);
	::closesocket(m_socket);
	m_socket = INVALID_SOCKET;
}