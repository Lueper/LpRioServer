#include "pch.h"

#include "LpNetCore.h"

LpRioCore::LpRioCore() {

}

LpRioCore::~LpRioCore() {

}

SOCKET LpRioCore::CreateRioSocket() {
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED | WSA_FLAG_REGISTERED_IO);
}

RIO_CQ LpRioCore::CreateRioCompletionQueue(RIO_EXTENSION_FUNCTION_TABLE& rio, DWORD cqSize, RIO_NOTIFICATION_COMPLETION* rioNotify) {
	return rio.RIOCreateCompletionQueue(cqSize, rioNotify);
}

RIO_RQ LpRioCore::CreateRioRequestQueue(RIO_EXTENSION_FUNCTION_TABLE& rio, SOCKET socket, DWORD maxPendingRecv, DWORD maxRecvBuffers, DWORD maxPendingSend, DWORD maxSendBuffers, RIO_CQ recvCQ, RIO_CQ sendCQ, PVOID context) {
	return rio.RIOCreateRequestQueue(socket, maxPendingRecv, maxRecvBuffers, maxPendingSend, maxSendBuffers, recvCQ, sendCQ, context);
}

RIO_BUFFERID LpRioCore::RegisterRioBuffer(RIO_EXTENSION_FUNCTION_TABLE& rio, char* buffer, DWORD size) {
	return rio.RIORegisterBuffer(buffer, size);
}

bool LpRioCore::LoadExFunctionTable(SOCKET socket, GUID guid, RIO_EXTENSION_FUNCTION_TABLE& rio) {
	DWORD bytes = 0;
	rio.cbSize = sizeof(rio);
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &rio, sizeof(rio), &bytes, NULL, NULL);
}

ULONG LpRioCore::PopRioEvent(RIO_EXTENSION_FUNCTION_TABLE& rio, RIO_CQ cq, RIORESULT* results, ULONG size) {
	return rio.RIODequeueCompletion(cq, results, size);
}

bool LpRioCore::NotifyRio(RIO_EXTENSION_FUNCTION_TABLE& rio, RIO_CQ cq) {
	return 0 == rio.RIONotify(cq);
}