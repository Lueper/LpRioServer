#pragma once

#include "LpSession.h"

struct LpIOContext;

class LpConnector : LpSession {
public:
	LpConnector();
	~LpConnector();

	virtual void ProcessIO(LpIOContext* ioContext, uint32_t bytes) override;

private:
	SOCKET m_socket = INVALID_SOCKET;
};