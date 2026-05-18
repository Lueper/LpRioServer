#include "pch.h"

#include "LpNetCore.h"

LpNetServer::LpNetServer() {

}

LpNetServer::~LpNetServer() {

}

bool LpNetServer::Init(ENetMode mode) {
	if (mode == ENetMode::RIO) {
		auto rioCore = std::make_shared<LpRioCore>();
		if (rioCore->Init())
			m_socketCore = std::move(rioCore);
	}

	// RIO 초기화 실패 시, IOCP로 폴백
	if (m_socketCore == nullptr) {
		auto iocpCore = std::make_shared<LpIocpCore>();
		if (iocpCore->Init())
			m_socketCore = std::move(iocpCore);
	}

	if (m_socketCore == nullptr)
		return false;

	m_eNetMode = m_socketCore->GetMode();
	return true;
}

void LpNetServer::Start() {
	m_running = true;

	m_socketCore->Start();
}

void LpNetServer::Stop() {
	m_running = false;

	m_socketCore->Stop();
}

void LpNetServer::Release() {

}