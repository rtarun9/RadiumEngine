#pragma once

#include "Pch.hpp"

#define ASSERT(condition, message)				\
{												\
do												\
{												\
	(condition) ? 0 : ErrorMessage(message);	\
}												\
while (0);										\
}												\
												
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
	exit(EXIT_FAILURE);
}

static inline void ErrorMessage(const std::wstring& error)
{
	MessageBoxW(0, error.c_str(), L"ERROR", MB_OK);
	exit(EXIT_FAILURE);
}

static inline std::wstring StringToWString(const std::string& string)
{
	std::wstring result(string.begin(), string.end());
	return result;
}

// NOTE : convert from enum to enum class in the future.
enum Keys
{
	W,
	A,
	S,
	D,
	AUp,
	ADown,
	ALeft,
	ARight,
	TotalKeyCount
};

// TODO : Find a way to mark this as either const / constexpr.
// NOTE:  Ignoring warning of 'prever enum class.
static std::unordered_map<int, Keys> INPUT_MAP =
{
	{'W', Keys::W},
	{'A', Keys::A},
	{'S', Keys::S},
	{'D', Keys::D},
	{VK_UP, Keys::AUp},
	{VK_LEFT, Keys::ALeft},
	{VK_DOWN, Keys::ADown},
	{VK_RIGHT, Keys::ARight}
};