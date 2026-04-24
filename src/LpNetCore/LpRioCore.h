#pragma once

class LpRioCore {
public:
	LpRioCore();
	~LpRioCore();

	static SOCKET CreateRioSocket();
	static RIO_CQ CreateRioCompletionQueue(RIO_EXTENSION_FUNCTION_TABLE& rio, DWORD cqSize, RIO_NOTIFICATION_COMPLETION* rioNotify);
	static RIO_RQ CreateRioRequestQueue(RIO_EXTENSION_FUNCTION_TABLE& rio, SOCKET socket, DWORD maxPendingRecv, DWORD maxRecvBuffers, DWORD maxPendingSend, DWORD maxSendBuffers, RIO_CQ recvCQ, RIO_CQ sendCQ, PVOID context);

	static RIO_BUFFERID RegisterRioBuffer(RIO_EXTENSION_FUNCTION_TABLE& rio, char* buffer, DWORD size);
	static bool LoadExFunctionTable(SOCKET socket, GUID guid, RIO_EXTENSION_FUNCTION_TABLE& rio);

	static ULONG PopRioEvent(RIO_EXTENSION_FUNCTION_TABLE& rio, RIO_CQ cq, RIORESULT* results, ULONG size);
	static bool NotifyRio(RIO_EXTENSION_FUNCTION_TABLE& rio, RIO_CQ cq);

public:
	RIO_EXTENSION_FUNCTION_TABLE m_rio = {};
	RIO_NOTIFICATION_COMPLETION m_rioNotify = {};
	RIO_CQ m_rioCQ = RIO_INVALID_CQ;
	RIO_RQ m_rioRQ = RIO_INVALID_RQ;
	RIO_BUFFERID m_recvBufId = RIO_INVALID_BUFFERID;
	RIO_BUFFERID m_sendBufId = RIO_INVALID_BUFFERID;
	char* m_recvPool = nullptr;
	char* m_sendPool = nullptr;

private:
};