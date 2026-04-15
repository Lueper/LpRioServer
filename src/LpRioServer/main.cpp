#include "LpServer.h"

int main(int argc, char* argv[]) {
	std::shared_ptr<LpServer> lpServer = std::make_shared<LpServer>();

	if (lpServer->Init() == false) {
		LOG_ERROR("Server initialize failed: %d", WSAGetLastError());
		return -1;
	}

	lpServer->Start();

	return 0;
}