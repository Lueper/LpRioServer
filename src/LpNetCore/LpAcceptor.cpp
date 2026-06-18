#include "pch.h"
#include "LpNetCore.h"

#include "LpAcceptor.h"

LpAcceptor::LpAcceptor() {
}

LpAcceptor::~LpAcceptor() {
}

HANDLE LpAcceptor::GetHandle() {
	return reinterpret_cast<HANDLE>(m_socket);
}

void LpAcceptor::ProcessIO(LpIOContext* ioContext, uint32_t bytes) {
}
