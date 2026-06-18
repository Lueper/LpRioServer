#pragma once

class LpConnector : LpSocketChannel {
public:
	LpConnector();
	~LpConnector();

	virtual HANDLE GetHandle() override;
	virtual void ProcessIO(LpIOContext* ioContext, uint32_t bytes = 0) override;

private:
	SOCKET m_socket = INVALID_SOCKET;
};