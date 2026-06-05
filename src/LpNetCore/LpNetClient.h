#pragma once

class LpNetClient {
public:
	LpNetClient();
	~LpNetClient();

	bool Init(ENetMode mode);
	void Start();
	void Stop();
	void Release();

private:
	std::vector<std::unique_ptr<std::thread>> m_ioThreadVec;
	std::shared_ptr<LpSocketCore> m_socketCore = nullptr;

	ENetMode m_eNetMode = ENetMode::None;
	std::atomic<bool> m_running = false;
};