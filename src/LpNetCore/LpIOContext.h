#pragma once

#include "LpGlobal.h"
#include "LpSession.h"

struct LpIOContext : public OVERLAPPED {
	LpIOContext(EIOType ioType) : m_ioType(ioType) {}

	EIOType m_ioType;
	std::shared_ptr<LpSession> m_session = nullptr;
};

struct LpAcceptContext : public LpIOContext {
	LpAcceptContext() : LpIOContext(EIOType::Accept) {}
};

struct LpConnectContext : public LpIOContext {
	LpConnectContext() : LpIOContext(EIOType::Connect) {}
};

struct LpDisconnectContext : public LpIOContext {
	LpDisconnectContext() : LpIOContext(EIOType::Disconnect) {}
};

struct LpRecvContext : public LpIOContext {
	LpRecvContext() : LpIOContext(EIOType::Recv) {}
};

struct LpSendContext : public LpIOContext {
	LpSendContext() : LpIOContext(EIOType::Send) {}
};