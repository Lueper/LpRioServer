#include "LpNetCore.h"

#include <csignal>

void RegisterSignal(LpNetServer* server) {
	static LpNetServer* lpServer = nullptr;
	lpServer = server;

	std::signal(SIGINT, [](int signal) {
		if (lpServer)
			lpServer->Stop();
	});
}

int main(int argc, char* argv[]) {
	LpNetServer* lpServer = new LpNetServer();

	RegisterSignal(lpServer);

	lpServer->Init(ENetMode::RIO);
	lpServer->Start();
	lpServer->Release();

	delete lpServer;
	return 0;
}