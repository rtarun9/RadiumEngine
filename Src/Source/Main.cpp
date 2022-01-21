#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include "../Include/d3dx12.h"

#include <wrl.h>

#include <algorithm>
#include <chrono>
#include <string>

#include "../Include/Helpers.hpp"

namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

// Application control variables
constexpr LPCWSTR WINDOW_CLASS_NAME = L"Base Window Class";

constexpr uint8_t NUMBER_OF_FRAMES = 2;

uint32_t g_ClientWidth = 1280;
uint32_t g_ClientHeight = 720;

bool g_IsInitialized = false;

// SwapChain control variables
bool g_Vsync = true;
bool g_TearingSupported = false;
bool g_FullScreen = false;

// Windows related variables
HWND g_WindowHandle = 0;
RECT g_WindowRect = {};

// DirectX Objects
wrl::ComPtr<ID3D12Device2> g_Device;

wrl::ComPtr<IDXGISwapChain4> g_SwapChain;
wrl::ComPtr<ID3D12Resource> g_BackBuffers[NUMBER_OF_FRAMES];

wrl::ComPtr<ID3D12GraphicsCommandList> g_CommandList;
wrl::ComPtr<ID3D12CommandAllocator> g_CommandAllocator[NUMBER_OF_FRAMES];
wrl::ComPtr<ID3D12CommandQueue> g_CommandQueue;

wrl::ComPtr<ID3D12DescriptorHeap> g_RTVDescriptorHeap;

UINT g_RTVDescriptorSize = 0;
UINT g_CurrentBackBufferIndex = 0;

// Sync objects
wrl::ComPtr<ID3D12Fence> g_Fence;
uint64_t g_FenceValue = 0; // Next value to signal command queue
uint64_t g_FenceFrameValues[NUMBER_OF_FRAMES] = {};	// The value needed to wait on for each frame
HANDLE g_FenceEvent; // OS evenet object used to stall thread 

// Function forward declerations
LRESULT CALLBACK WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

void EnableDebugLayer()
{
#ifdef _DEBUG
	wrl::ComPtr<ID3D12Debug> debugInterface;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
#endif
}

void RegisterAppWindowClass(HINSTANCE instance, LPCWSTR className)
{
	WNDCLASSEXW windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.lpszClassName = className;
	windowClass.lpszMenuName = nullptr;
	windowClass.hInstance = instance;
	windowClass.hIcon = nullptr;
	windowClass.hIconSm = nullptr;
	windowClass.hCursor = nullptr;

	if (!(::RegisterClassExW(&windowClass)))
	{
		ErrorMessage(L"Failed to register window class");
	}
}


HWND CreateAppWindow(HINSTANCE instance, LPCWSTR windowTitle, LPCWSTR windowClassName, uint32_t width, uint32_t height)
{
	int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

	RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, 0);

	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	// Center screen by calculation top left corner position : Clamp to 0, 0 as well.
	int windowXPos = std::max<int>(0, (screenWidth - windowWidth) / 2);
	int windowYPos = std::max<int>(0, (screenHeight - windowHeight) / 2);

	HWND windowHandle = ::CreateWindowExW(0, windowClassName, windowTitle,
		WS_OVERLAPPEDWINDOW, windowXPos, windowYPos, windowWidth, windowHeight, 0, 0, instance, 0);

	if (!windowHandle)
	{
		ErrorMessage(L"Failed to create window");
	}

	return windowHandle;
}

// Get compatable adapter (GPU)
wrl::ComPtr<IDXGIAdapter4> GetAdapter()
{
	wrl::ComPtr<IDXGIFactory4> factory;
	UINT createFactoryFlags = 0;

#if _DEBUG
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory)));

	wrl::ComPtr<IDXGIAdapter1> adapter1;
	wrl::ComPtr<IDXGIAdapter4> adapter4;
	
	// Find adapter with height video memory, can create d3d12 device and one that is not a software adapter.
	SIZE_T maxDedicatedVideoMemory = 0;
	for (UINT i = 0; factory->EnumAdapters1(i, &adapter1) != DXGI_ERROR_NOT_FOUND; i++)
	{
		DXGI_ADAPTER_DESC1 adapterDesc = {};
		adapter1->GetDesc1(&adapterDesc);

		if ((adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
			(SUCCEEDED(D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr)) &&
			(adapterDesc.DedicatedVideoMemory > maxDedicatedVideoMemory)))
		{
			maxDedicatedVideoMemory = adapterDesc.DedicatedVideoMemory;
			ThrowIfFailed(adapter1.As(&adapter4));
		}
	}

	return adapter4;
}

wrl::ComPtr<ID3D12Device2> CreateDevice(wrl::ComPtr<IDXGIAdapter4> adapter)
{
	wrl::ComPtr<ID3D12Device2> device;
	ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device)));

#if _DEBUG
	wrl::ComPtr<ID3D12InfoQueue> infoQueue;
	if (SUCCEEDED(device.As(&infoQueue)))
	{
		infoQueue->GetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR);
		infoQueue->GetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING);
		infoQueue->GetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION);

		// Ignore warnings with severity level of info. 
		D3D12_MESSAGE_SEVERITY ignoreSeverities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO
		};

		D3D12_INFO_QUEUE_FILTER infoQueueFilter = {};
		infoQueueFilter.DenyList.NumSeverities = _countof(ignoreSeverities);
		infoQueueFilter.DenyList.pSeverityList = ignoreSeverities;

		ThrowIfFailed(infoQueue->PushStorageFilter(&infoQueueFilter));
	}

#endif

	return device;
}

wrl::ComPtr<ID3D12CommandQueue> CreateCommandQueue(wrl::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT)
{
	wrl::ComPtr<ID3D12CommandQueue> commandQueue;

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.NodeMask = 0;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Type = type;

	ThrowIfFailed(device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue)));

	return commandQueue;
}

bool CheckTearingSupport()
{
	bool allowTearing = false;

	wrl::ComPtr<IDXGIFactory5> factory;
	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
	{
		if (FAILED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
		{
			allowTearing = false;
		}
	}

	return allowTearing;
}

wrl::ComPtr<IDXGISwapChain4> CreateSwapChain(HWND windowHandle, wrl::ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height)
{
	wrl::ComPtr<IDXGISwapChain4> swapChain4;
	wrl::ComPtr<IDXGIFactory4> factory;

	UINT createFactoryFlags = 0;
#if _DEBUG
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferCount = NUMBER_OF_FRAMES;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	wrl::ComPtr<IDXGISwapChain1> swapChain1;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(commandQueue.Get(), windowHandle, &swapChainDesc, nullptr, nullptr, &swapChain1));

	// Disable functionality for full screen toggle via ALT + ENTER
	ThrowIfFailed(factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER));
	
	ThrowIfFailed(swapChain1.As(&swapChain4));

	return swapChain4;
}