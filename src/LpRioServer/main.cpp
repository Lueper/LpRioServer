#include "LpServer.h"

int main(int argc, char* argv[]) {
	LpNetServer* lpServer = new LpNetServer();

	lpServer->Init(ENetMode::RIO);
	lpServer->Start();

	return 0;
}