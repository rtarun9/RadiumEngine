#pragma once

// Dont include the mostly unused stuff from the windows header.
#define WIN32_LEAN_AND_MEAN

#define NOMINMAX

#ifndef UNICODE
#define UNICODE
#endif

#include <Windows.h>

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <wrl.h>

#include <algorithm>
#include <chrono>
#include <string>
#include <vector>

#include "Helpers.hpp"

namespace wrl = Microsoft::WRL;
namespace dx = DirectX;
