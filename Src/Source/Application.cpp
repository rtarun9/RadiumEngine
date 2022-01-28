#include "Pch.hpp"

#include "Application.hpp"
#include "Engine.hpp"

#include "imgui.h"
#include <imgui_impl_win32.h>

HWND Application::s_WindowHandle = 0;

std::chrono::high_resolution_clock Application::s_Clock = {};
std::chrono::high_resolution_clock::time_point Application::s_PreviousFrameTime = {};
std::chrono::high_resolution_clock::time_point Application::s_CurrentFrameTime = {};
double Application::s_DeltaTime = 0.0f;

// Forward declare message handler from imgui_impl_win32.cpp.
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


int Application::Run(std::shared_ptr<Engine> engine, HINSTANCE instance)
{
	// Register window class and create window class.
	WNDCLASSEXW windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = WINDOW_CLASS_NAME;
	windowClass.lpszMenuName = nullptr;
	windowClass.hCursor = nullptr;
	windowClass.hbrBackground = nullptr;
	windowClass.hIcon = nullptr;
	windowClass.hIconSm = nullptr;

	if (!::RegisterClassExW(&windowClass))
	{
		ErrorMessage(L"Failed to register window class");
	}

	RECT windowRect = { 0, 0, static_cast<LONG>(engine->GetWidth()), static_cast<LONG>(engine->GetHeight())};
	::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	s_WindowHandle = ::CreateWindowExW(0,
		WINDOW_CLASS_NAME, engine->GetTitle().c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
		0, 0, instance, engine.get());

	if (!s_WindowHandle)
	{
		ErrorMessage(L"Failed to create window");
	}

	engine->OnInit();

	::ShowWindow(s_WindowHandle, SW_SHOW);

	// Main game loop
	MSG message = {};
	while (message.message != WM_QUIT)
	{
		s_CurrentFrameTime = s_Clock.now();
		s_DeltaTime = (s_CurrentFrameTime - s_PreviousFrameTime).count() * 1e-9;
		s_PreviousFrameTime = s_CurrentFrameTime;

		if (::PeekMessageW(&message, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&message);
			::DispatchMessageW(&message);
		}

		engine->OnUpdate(s_DeltaTime);
		engine->OnRender();
	}

	engine->OnDestroy();
	
	UnregisterClassW(WINDOW_CLASS_NAME, instance);
	return static_cast<int>(message.wParam);
}

HWND Application::GetWindowHandle()
{
	return s_WindowHandle;
}

double Application::GetDeltaTime()
{
	return s_DeltaTime;
}

LRESULT CALLBACK Application::WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(windowHandle, message, wParam, lParam))
	{
		return true;
	}

	Engine* engine = reinterpret_cast<Engine*>(GetWindowLongPtrW(windowHandle, GWLP_USERDATA));

	switch (message)
	{
	case WM_CREATE:
	{
		// Save the engine instance passed as last parameter to CreateWindow.
		LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCTW>(lParam);
		SetWindowLongPtrW(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
	}break;

	case WM_DESTROY:
	{
		::PostQuitMessage(0);
	}break;

	case WM_KEYUP:
	{
		engine->OnKeyUp(static_cast<uint32_t>(wParam));
	}break;

	case WM_KEYDOWN:
	{
		engine->OnKeyDown(static_cast<uint32_t>(wParam));

		switch (wParam)
		{
		case VK_ESCAPE:
		{
			::DestroyWindow(windowHandle);
		}break;
		}
	}break;

	default:
	{
		return ::DefWindowProcW(windowHandle, message, wParam, lParam);
	}break;
	}

	return 0;
}
