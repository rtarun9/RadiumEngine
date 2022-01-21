#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <exception>

static inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}

static inline void ErrorMessage(LPCWSTR error)
{
	MessageBoxW(0, error, L"ERROR", MB_OK);
}