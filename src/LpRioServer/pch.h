#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

// mswsock.h보다 먼저 선언
#include <ws2tcpip.h>

#include <concurrent_queue.h>
#include <mswsock.h>
#include <sys/timeb.h>
#include <windows.h>
#include <winsock2.h>

#include <atomic>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <stack>
#include <string>
#include <thread>
#include <vector>

#include "LpNetCore.h"

#include "LpServer.h"
#include "Utility/LpSingleton.h"