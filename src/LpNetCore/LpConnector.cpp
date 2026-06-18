#include "pch.h"
#include "LpNetCore.h"

#include "LpConnector.h"

LpConnector::LpConnector() {
}

LpConnector::~LpConnector() {
}

HANDLE LpConnector::GetHandle() {
	return reinterpret_cast<HANDLE>(m_socket);
}

void LpConnector::ProcessIO(LpIOContext* ioContext, uint32_t bytes) {
}
