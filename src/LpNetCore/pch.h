#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

// mswsock.h보다 먼저 선언
#include <ws2tcpip.h>

#include <atomic>
#include <concurrent_queue.h>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mswsock.h>
#include <mutex>
#include <sstream>
#include <stack>
#include <string>
#include <sys/timeb.h>
#include <thread>
#include <vector>
#include <windows.h>
#include <winsock2.h>

#include "LpNetCore.h"