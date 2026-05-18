#pragma once

class LpIocpCore : public LpSocketCore {
public:
	LpIocpCore();
	~LpIocpCore();

	bool Init() override;
	void Start() override;
	void Stop() override;

	ENetMode GetMode() override { return ENetMode::IOCP; };

private:
	void Run();

	void PostAccept();

public:
	SOCKET m_socket = INVALID_SOCKET;
	HANDLE m_iocp = nullptr;
	OVERLAPPED m_overlapped = {};

private:
	std::vector<std::unique_ptr<std::thread>> m_ioThreadVec;
};