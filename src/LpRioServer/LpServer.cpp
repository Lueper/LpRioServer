#include "LpServer.h"

LpServer::LpServer() {
	m_socket = INVALID_SOCKET;
}

LpServer::~LpServer() {

}

bool LpServer::Init() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return false;
	}

	m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_socket == INVALID_SOCKET) {
		return false;
	}

	DWORD bytes;
	GUID acceptId = WSAID_ACCEPTEX;
	int result = WSAIoctl(m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
						  &acceptId, sizeof(acceptId), &m_lpfnAcceptEx, sizeof(m_lpfnAcceptEx), &bytes, NULL, NULL);
	if (result != 0) {
		closesocket(m_socket);
		return false;
	}

	GUID sockaddrsId = WSAID_GETACCEPTEXSOCKADDRS;
	result = WSAIoctl(m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
					  &sockaddrsId, sizeof(sockaddrsId), &m_lpfnGetAcceptExSockaddrs, sizeof(m_lpfnGetAcceptExSockaddrs), &bytes, NULL, NULL);
	if (result != 0) {
		closesocket(m_socket);
		return false;
	}

	m_rio.cbSize = sizeof(RIO_EXTENSION_FUNCTION_TABLE);
	GUID rioId = WSAID_MULTIPLE_RIO;
	result = WSAIoctl(m_socket, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER,
					  &rioId, sizeof(rioId), &m_rio, sizeof(m_rio), &bytes, NULL, NULL);
	if (result != 0) {
		closesocket(m_socket);
		return false;
	}

	return true;
}

void LpServer::Start() {
	m_running = true;

	LOG_INFO("Server Start");
}

void LpServer::Stop() {
	m_running = false;

	LOG_INFO("Server Stop");
}

void LpServer::Release() {
	LOG_INFO("Server Release");
}

void LpServer::Run() {

}