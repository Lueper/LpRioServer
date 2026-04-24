#pragma once

class LpNetServer {
public:
	LpNetServer();
	~LpNetServer();

	void Init(ENetMode mode);
	void Start();
	void Stop();
	void Release();
private:
	void Run();

	bool InitRioCore();

	LpIocpCore* m_iocpCore;
	LpRioCore* m_rioCore;

	SOCKET m_socket = INVALID_SOCKET;

	std::atomic<bool> m_running = false;
};