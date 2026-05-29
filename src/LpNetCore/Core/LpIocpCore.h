#pragma once

class LpIocpCore : public LpSocketCore {
public:
	LpIocpCore();
	~LpIocpCore();

	bool Init() override;
	void Start(int threadCount) override;
	void Run() override;
	void Stop() override;

	ENetMode GetMode() override { return ENetMode::IOCP; };

private:
	void PostAccept();
	void OnAccept(AcceptContext* actx);

	void ProcessRecv(RIORESULT result);
	void ProcessSend(RIORESULT result);

public:
	SOCKET m_socket = INVALID_SOCKET;
	HANDLE m_iocp = nullptr;
	OVERLAPPED m_overlapped = {};

private:
};