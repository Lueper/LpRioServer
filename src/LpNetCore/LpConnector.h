#pragma once

struct LpIOContext;
class LpSession;

class LpConnector : LpSession {
public:
	LpConnector();
	~LpConnector();

	virtual void ProcessIO(LpIOContext* ioContext, uint32_t bytes) override;

private:
	SOCKET m_socket = INVALID_SOCKET;
};