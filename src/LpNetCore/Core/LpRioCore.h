#pragma once

class LpRioCore : public LpSocketCore {
public:
	LpRioCore();
	~LpRioCore();

	bool Init() override;
	void Start() override;

	ENetMode GetMode() override { return ENetMode::RIO; };

public:
	SOCKET m_socket = INVALID_SOCKET;
	HANDLE m_iocp = nullptr;
	OVERLAPPED m_overlapped = {};
	RIO_EXTENSION_FUNCTION_TABLE m_rio = {};
	RIO_NOTIFICATION_COMPLETION m_rioNotify = {};
	RIO_CQ m_rioCQ = RIO_INVALID_CQ;
	RIO_RQ m_rioRQ = RIO_INVALID_RQ;
	RIO_BUFFERID m_recvBufferId = RIO_INVALID_BUFFERID;
	RIO_BUFFERID m_sendBufferId = RIO_INVALID_BUFFERID;
	char* m_recvPool = nullptr;
	char* m_sendPool = nullptr;

private:
	std::atomic<bool> m_running = false;
};