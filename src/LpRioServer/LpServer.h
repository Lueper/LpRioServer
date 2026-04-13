#pragma once

#include <iostream>

#include "yaml-cpp/yaml.h"

class LpServer {
public:
	LpServer();
	~LpServer();

private:
	std::atomic<bool> m_running;
};