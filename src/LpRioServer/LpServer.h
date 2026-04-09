#pragma once

class LpServer {
public:
	LpServer();
	~LpServer();

private:
	std::atomic<bool> m_running;
};