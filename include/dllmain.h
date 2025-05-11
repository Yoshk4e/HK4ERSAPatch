#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Windows and standard library
#include <Windows.h>
#include <cstdint>
#include <thread>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

// Third-party libraries
#include <MinHook.h>

// Project headers
#include <Util.h>
#include <ccp_blocker.hpp>
#include <security.hpp>
#include <misc.hpp>
#include <http.hpp>
#include <ModuleManager.hpp>
#include <interceptor.hpp>

// Thread entry point
DWORD WINAPI MainThread(LPVOID lpReserved);
