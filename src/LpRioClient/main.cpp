#include "LpNetCore.h"

#include <csignal>

void RegisterSignal(LpNetClient* client) {
	static LpNetClient* lpClient = nullptr;
	lpClient = client;

	std::signal(SIGINT, [](int signal) {
		if (lpClient)
			lpClient->Stop();
	});
}

int main(int argc, char* argv[]) {
	LpNetClient* lpClient = new LpNetClient();

	RegisterSignal(lpClient);

	lpClient->Init(ENetMode::IOCP);
	lpClient->Start();
	lpClient->Release();

	delete lpClient;
	return 0;
}