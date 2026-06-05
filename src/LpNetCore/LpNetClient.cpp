#include "pch.h"

#include "LpNetCore.h"

LpNetClient::LpNetClient() {

}

LpNetClient::~LpNetClient() {

}

bool LpNetClient::Init(ENetMode mode) {
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

void LpNetClient::Start() {
	m_running = true;

	LOG_INFO("Client Started");

	int threadCount = 1;
	m_socketCore->StartConnect(threadCount);

	for (int i = 0; i < threadCount; i++) {
		std::unique_ptr<std::thread> thread = std::make_unique<std::thread>([this] {
			m_socketCore->Run();
		});
		m_ioThreadVec.push_back(std::move(thread));
	}

	for (int i = 0; i < threadCount; i++) {
		std::unique_ptr<std::thread> thread = std::make_unique<std::thread>([this] {
			m_socketCore->RunClient();
		});
		m_ioThreadVec.push_back(std::move(thread));
	}

	for (auto& thread : m_ioThreadVec) {
		if (thread->joinable())
			thread->join();
	}
	m_ioThreadVec.clear();
}

void LpNetClient::Stop() {
	m_running = false;

	LOG_INFO("Client Stopping...");

	m_socketCore->Stop();
}

void LpNetClient::Release() {
	LOG_INFO("Client Released");
}