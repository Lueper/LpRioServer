#pragma once

class LpSession {
public:
	LpSession();
	LpSession(SOCKET socket);
	~LpSession();

	void Disconnect();

	SOCKET GetSocket() { return m_socket; };

private:
	SOCKET m_socket = INVALID_SOCKET;

	std::atomic<bool> m_connected = false;

	LpBuffer* m_recvBuffer = nullptr;
	LpBuffer* m_sendBuffer = nullptr;
};