#pragma once

struct LpIOContext;
class LpSession;

class LpAcceptor : LpSession {
public:
	LpAcceptor();
	~LpAcceptor();

	virtual void ProcessIO(LpIOContext* ioContext, uint32_t bytes) override;

private:
	SOCKET m_socket = INVALID_SOCKET;
};