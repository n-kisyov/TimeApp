#pragma once

// Always include winsock2.h BEFORE windows.h to avoid conflicts
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

// Include winsock2 FIRST
#include <winsock2.h>
#include <ws2tcpip.h>

// Then include windows.h
#include <windows.h>

// DirectX headers
#include <d3d11.h>
#include <dxgi1_2.h>

// Suppress deprecated warnings
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32.lib")
