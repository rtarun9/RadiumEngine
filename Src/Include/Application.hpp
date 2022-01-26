#pragma once

#include "Pch.hpp"

class Engine;

class Application
{
public:
	[[nodiscard]] static int Run(std::shared_ptr<Engine> engine, HINSTANCE instance);
	[[nodiscard]] static HWND GetWindowHandle();

private:
	static LRESULT CALLBACK WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

private:
	static constexpr LPCWSTR WINDOW_CLASS_NAME = L"Base Window Class";

	static HWND s_WindowHandle;

	static std::chrono::high_resolution_clock s_Clock;
	static std::chrono::high_resolution_clock::time_point s_PreviousFrameTime;
	static std::chrono::high_resolution_clock::time_point s_CurrentFrameTime;
	static double s_DeltaTime;

};