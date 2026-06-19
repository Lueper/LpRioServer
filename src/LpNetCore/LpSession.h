#pragma once

struct LpIOContext;
class LpBuffer;

class LpSession : public std::enable_shared_from_this<LpSession> {
public:
	LpSession();
	LpSession(SOCKET socket);
	~LpSession();

	virtual void ProcessIO(LpIOContext* ioContext, uint32_t bytes) = 0;

	void Disconnect();

	HANDLE GetHandle() { return reinterpret_cast<HANDLE>(m_socket); }
	SOCKET GetSocket() { return m_socket; }

	template <typename T>
	std::shared_ptr<T> ToShared() {
		return std::static_pointer_cast<T>(shared_from_this());
	}

private:
	SOCKET m_socket = INVALID_SOCKET;
	LpBuffer* m_recvBuffer = nullptr;
	LpBuffer* m_sendBuffer = nullptr;

	std::atomic<bool> m_connected = false;
};