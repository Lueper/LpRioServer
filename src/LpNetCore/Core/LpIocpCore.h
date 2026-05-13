#pragma once

class LpIocpCore : public LpSocketCore {
public:
	LpIocpCore();
	~LpIocpCore();

	bool Init() override;
	void Start() override;
	void Run(int threadCount) override;

	ENetMode GetMode() override { return ENetMode::IOCP; };

private:
	void PostAccept();
	void Process();

public:
	SOCKET m_socket = INVALID_SOCKET;
	HANDLE m_iocp = nullptr;
	OVERLAPPED m_overlapped = {};

private:
	std::vector<std::thread*> m_ioThreadVec;
	std::atomic<bool> m_running = false;
};