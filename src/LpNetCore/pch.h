#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

#include "framework.h"

#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>

#include <iostream>
#include <vector>
#include <string>
#include <memory>