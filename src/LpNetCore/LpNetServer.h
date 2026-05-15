#pragma once

class LpNetServer {
public:
	LpNetServer();
	~LpNetServer();

	void Init();
	bool Init(ENetMode mode);
	void Start();
	void Stop();
	void Release();

private:
	std::shared_ptr<LpSocketCore> m_socketCore = nullptr;
	SOCKET m_socket	= INVALID_SOCKET;

	ENetMode m_eNetMode = ENetMode::None;
	std::atomic<bool> m_running	= false;
};