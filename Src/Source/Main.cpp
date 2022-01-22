#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
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

wrl::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(wrl::ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptorCount)
{
	wrl::ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Type = type;
	descriptorHeapDesc.NumDescriptors = descriptorCount;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ThrowIfFailed(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap)));

	return descriptorHeap;
}

void CreateRenderTargetViews(wrl::ComPtr<ID3D12Device2> device, wrl::ComPtr<IDXGISwapChain4> swapChain, wrl::ComPtr<ID3D12DescriptorHeap> descriptorHeap)
{
	auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < NUMBER_OF_FRAMES; i++)
	{
		wrl::ComPtr<ID3D12Resource> backBuffer;
		ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

		device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
		g_BackBuffers[i] = backBuffer;

		rtvHandle.Offset(rtvDescriptorSize);
	}
}

wrl::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(wrl::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
{
	wrl::ComPtr<ID3D12CommandAllocator> commandAllocator;
	ThrowIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));

	return commandAllocator;
}

wrl::ComPtr<ID3D12GraphicsCommandList> CreateCommandList(wrl::ComPtr<ID3D12Device2> device, wrl::ComPtr<ID3D12CommandAllocator> commandAllocator, D3D12_COMMAND_LIST_TYPE type)
{
	wrl::ComPtr<ID3D12GraphicsCommandList> commandList;

	ThrowIfFailed(device->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
	
	// In render loop fisrt operation performed on allocator and list is reset, which needs the list to be closed before hand.
	ThrowIfFailed(commandList->Close());

	return commandList;
}

wrl::ComPtr<ID3D12Fence> CreateFence(wrl::ComPtr<ID3D12Device2> device)
{
	wrl::ComPtr<ID3D12Fence> fence;
	ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

	return fence;
}

HANDLE CreateEventHandle()
{
	HANDLE fenceEvent;
	
	fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!fenceEvent)
	{
		ErrorMessage(L"Failed to create fence event");
	}

	return fenceEvent;
}

uint64_t Signal(wrl::ComPtr<ID3D12CommandQueue> commandQueue, wrl::ComPtr<ID3D12Fence> fence, uint64_t& fenceValue)
{
	uint64_t fenceValueToSignal = fenceValue++;
	ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceValueToSignal));

	// Value CPU thread should wait on until GPU inflight frame has value >= this.
	return fenceValueToSignal;
}

void WaitForFenceValue(wrl::ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent, std::chrono::milliseconds duration = std::chrono::milliseconds::max())
{
	if (fence->GetCompletedValue() < fenceValue)
	{
		ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent));
		::WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
	}
}

void Flush(wrl::ComPtr<ID3D12CommandQueue> commandQueue, wrl::ComPtr<ID3D12Fence> fence, uint64_t& fenceValue, HANDLE fenceEvent)
{
	uint64_t fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
	WaitForFenceValue(fence, fenceValueForSignal, fenceEvent);
}

void Update()
{
	static uint64_t frameCounter = 0;
	static double elapsedSeconds = 0.0f;
	static std::chrono::high_resolution_clock clock;

	static auto previousFrameTime= clock.now();

	frameCounter++;
	auto time = clock.now();
	auto deltaTime = time - previousFrameTime;
	time = previousFrameTime;

	elapsedSeconds += deltaTime.count() * 1e-9;
	if (elapsedSeconds > 1.0f)
	{
		char buffer[500];
		auto fps = frameCounter / elapsedSeconds;
		sprintf_s(buffer, 500, "FPS : %f\n", fps);
		OutputDebugStringA(buffer);

		frameCounter = 0;
		elapsedSeconds = 0.0f;
	}
}

void Render()
{
	auto commandAllocator = g_CommandAllocator[g_CurrentBackBufferIndex];
	auto backBuffer = g_BackBuffers[g_CurrentBackBufferIndex];

	commandAllocator->Reset();
	g_CommandList->Reset(commandAllocator.Get(), nullptr);

	CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	g_CommandList->ResourceBarrier(1, &resourceBarrier);

	float clearColor[] = { 0.4f, 0.8f, 0.7f, 1.0f };
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), g_CurrentBackBufferIndex, g_RTVDescriptorSize);
	g_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0u, nullptr);

	resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	g_CommandList->ResourceBarrier(1, &resourceBarrier);

	ThrowIfFailed(g_CommandList->Close());

	ID3D12CommandList* const commandLists[] =
	{
		g_CommandList.Get()
	};


	g_CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	UINT syncInterval = g_Vsync ? 1 : 0;
	UINT presentFlag = g_TearingSupported && !g_Vsync ? DXGI_PRESENT_ALLOW_TEARING : 0;

	ThrowIfFailed(g_SwapChain->Present(syncInterval, presentFlag));

	g_FenceFrameValues[g_CurrentBackBufferIndex] = Signal(g_CommandQueue, g_Fence, g_FenceValue);

	g_CurrentBackBufferIndex = g_SwapChain->GetCurrentBackBufferIndex();

	WaitForFenceValue(g_Fence, g_FenceFrameValues[g_CurrentBackBufferIndex], g_FenceEvent);
}

void Resize(uint32_t width, uint32_t height)
{
	if (g_ClientHeight != height || g_ClientWidth != width)
	{
		g_ClientHeight = std::max<uint32_t>(height, 1u);
		g_ClientWidth = std::max<uint32_t>(width, 1u);

		Flush(g_CommandQueue, g_Fence, g_FenceValue, g_FenceEvent);

		for (int i = 0; i < NUMBER_OF_FRAMES; i++)
		{
			g_BackBuffers[i].Reset();
			g_FenceFrameValues[i] = g_FenceFrameValues[g_CurrentBackBufferIndex];
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		ThrowIfFailed(g_SwapChain->GetDesc(&swapChainDesc));
		ThrowIfFailed(g_SwapChain->ResizeBuffers(NUMBER_OF_FRAMES, g_ClientWidth, g_ClientHeight, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

		g_CurrentBackBufferIndex = g_SwapChain->GetCurrentBackBufferIndex();

		CreateRenderTargetViews(g_Device, g_SwapChain, g_RTVDescriptorHeap);
	}
}

LRESULT CALLBACK WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (g_IsInitialized)
	{
		switch (message)
		{
		case WM_PAINT:
		{
			Update();
			Render();
		}break;

		case WM_SIZE:
		{
			RECT clientRect = {};
			::GetClientRect(g_WindowHandle, &clientRect);

			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;

			Resize(width, height);
		}break;

		case WM_DESTROY:
		{
			::PostQuitMessage(0);
		}break;

		default:
		{
			return ::DefWindowProcW(windowHandle, message, wParam, lParam);
		}
		}
	}
	else
	{
		return ::DefWindowProcW(windowHandle, message, wParam, lParam);
	}

	return 0;
}

int CALLBACK wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
	EnableDebugLayer();

	g_TearingSupported = CheckTearingSupport();

	RegisterAppWindowClass(instance, WINDOW_CLASS_NAME);
	g_WindowHandle = CreateAppWindow(instance, L"<Radium Engine>", WINDOW_CLASS_NAME, g_ClientWidth, g_ClientHeight);

	wrl::ComPtr<IDXGIAdapter4> adapter = GetAdapter();

	g_Device = CreateDevice(adapter);

	g_CommandQueue = CreateCommandQueue(g_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);

	g_SwapChain = CreateSwapChain(g_WindowHandle, g_CommandQueue, g_ClientWidth, g_ClientHeight);

	g_CurrentBackBufferIndex = g_SwapChain->GetCurrentBackBufferIndex();

	g_RTVDescriptorHeap = CreateDescriptorHeap(g_Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1u);
	g_RTVDescriptorSize = g_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CreateRenderTargetViews(g_Device, g_SwapChain, g_RTVDescriptorHeap);

	for (int i = 0; i < NUMBER_OF_FRAMES; i++)
	{
		g_CommandAllocator[i] = CreateCommandAllocator(g_Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	}

	g_CommandList = CreateCommandList(g_Device, g_CommandAllocator[g_CurrentBackBufferIndex], D3D12_COMMAND_LIST_TYPE_DIRECT);

	g_Fence = CreateFence(g_Device);
	g_FenceEvent = CreateEventHandle();

	g_IsInitialized = true;

	::ShowWindow(g_WindowHandle, SW_SHOW);

	MSG message = { };
	while (message.message != WM_QUIT)
	{
		if (::PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&message);
			::DispatchMessage(&message);
		}
	}

	Flush(g_CommandQueue, g_Fence, g_FenceValue, g_FenceEvent);

	::CloseHandle(g_FenceEvent);

	return 0;
}