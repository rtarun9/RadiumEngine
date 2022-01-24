#include "Pch.hpp"

constexpr LPCWSTR WINDOW_CLASS_NAME = L"Base Window Class";
LPCWSTR g_WindowTitle = L"Radium Engine";

HWND g_WindowHandle = 0;

LONG g_ClientWidth = 1080;
LONG g_ClientHeight = 720;

const bool g_EnableVsync = TRUE;

wrl::ComPtr<ID3D11Device> g_Device;
wrl::ComPtr<ID3D11DeviceContext> g_DeviceContext;
wrl::ComPtr<IDXGISwapChain> g_SwapChain;

wrl::ComPtr<ID3D11DepthStencilView> g_DepthStencilView;
wrl::ComPtr<ID3D11Texture2D> g_DepthStencilBuffer;
wrl::ComPtr<ID3D11RenderTargetView> g_RenderTargetView;

wrl::ComPtr<ID3D11DepthStencilState> g_DepthStencilState;
wrl::ComPtr<ID3D11RasterizerState> g_RasterizerState;

D3D11_VIEWPORT g_Viewport = {};

wrl::ComPtr<ID3D11InputLayout> g_InputLayout;
wrl::ComPtr<ID3D11Buffer> g_VertexBuffer;
wrl::ComPtr<ID3D11Buffer> g_IndexBuffer;

wrl::ComPtr<ID3D11VertexShader> g_VertexShader;
wrl::ComPtr<ID3D11PixelShader> g_PixelShader;

enum ConstantBuffers
{
	CB_Applcation,
	CB_Frame,
	CB_Object,
	ConstantBufferCount
};

wrl::ComPtr<ID3D11Buffer> g_ConstantBuffers[ConstantBuffers::ConstantBufferCount] = {};

dx::XMMATRIX g_ModelMatrix;
dx::XMMATRIX g_ViewMatrix;
dx::XMMATRIX g_ProjectionMatrix;

struct Vertex
{
	dx::XMFLOAT3 position;
	dx::XMFLOAT3 color;
};

Vertex g_CubeVertices[8] =
{
	{ dx::XMFLOAT3(-1.0f, -1.0f, -1.0f),	dx::XMFLOAT3(0.0f, 0.0f, 0.0f) },
	{ dx::XMFLOAT3(-1.0f,  1.0f, -1.0f),	dx::XMFLOAT3(0.0f, 1.0f, 0.0f) },
	{ dx::XMFLOAT3(1.0f,  1.0f, -1.0f),		dx::XMFLOAT3(1.0f, 1.0f, 0.0f) },
	{ dx::XMFLOAT3(1.0f, -1.0f, -1.0f),		dx::XMFLOAT3(1.0f, 0.0f, 0.0f) },
	{ dx::XMFLOAT3(-1.0f, -1.0f,  1.0f),	dx::XMFLOAT3(0.0f, 0.0f, 1.0f) },
	{ dx::XMFLOAT3(-1.0f,  1.0f,  1.0f),	dx::XMFLOAT3(0.0f, 1.0f, 1.0f) },
	{ dx::XMFLOAT3(1.0f,  1.0f,  1.0f),		dx::XMFLOAT3(1.0f, 1.0f, 1.0f) },
	{ dx::XMFLOAT3(1.0f, -1.0f,  1.0f),		dx::XMFLOAT3(1.0f, 0.0f, 1.0f) } 
};

uint32_t g_CubeIndices[36] =
{
	0, 1, 2, 0, 2, 3,
	4, 6, 5, 4, 7, 6,
	4, 5, 1, 4, 1, 0,
	3, 2, 6, 3, 6, 7,
	1, 5, 6, 1, 6, 2,
	4, 0, 3, 4, 3, 7
};

std::chrono::high_resolution_clock g_Clock;
std::chrono::high_resolution_clock::time_point g_PreviousFrameTime;
std::chrono::high_resolution_clock::time_point g_CurrentFrameTime;
double g_DeltaTime;

LRESULT CALLBACK WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
	{
		::PostQuitMessage(0);
	}break;

	case WM_KEYDOWN:
	{
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
		::DefWindowProcW(windowHandle, message, wParam, lParam);
	}break;
	}

	return ::DefWindowProcW(windowHandle, message, wParam, lParam);
}

void CreateAppWindow(HINSTANCE instance)
{
	WNDCLASSEXW windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
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

	RECT windowRect = { 0, 0, g_ClientWidth, g_ClientHeight };
	::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	g_WindowHandle = ::CreateWindowExW(0, WINDOW_CLASS_NAME, g_WindowTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight,
		0, 0, instance, 0);

	if (!g_WindowHandle)
	{
		ErrorMessage(L"Failed to create window");
	}
}

wrl::ComPtr<ID3DBlob> LoadShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& shaderProfile)
{
	wrl::ComPtr<ID3DBlob> shaderBlob;
	wrl::ComPtr<ID3DBlob> errorBlob;

	UINT flags = 0;

#if _DEBUG
	flags = D3DCOMPILE_DEBUG;
#endif

	HRESULT hr = D3DCompileFromFile(fileName.c_str(), nullptr, nullptr, entryPoint.c_str(), shaderProfile.c_str(), flags, 0, &shaderBlob, &errorBlob);
	
	if (FAILED(hr))
	{
		if (hr == 0x80070003)
		{
			ErrorMessage(std::string("Could not find file"));
		}
		if (errorBlob.Get())
		{
			std::string errorMessage = (char*)errorBlob->GetBufferPointer();
			ErrorMessage(errorMessage);

			return nullptr;
		}
	}
	
	return shaderBlob;
}

void LoadContent()
{
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = sizeof(g_CubeVertices);
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexSubresourceData = {};
	vertexSubresourceData.pSysMem = g_CubeVertices;

	ThrowIfFailed(g_Device->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &g_VertexBuffer));

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.ByteWidth = sizeof(g_CubeIndices);
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indicesSubresouceData = {};
	indicesSubresouceData.pSysMem = g_CubeIndices;

	ThrowIfFailed(g_Device->CreateBuffer(&indexBufferDesc, &indicesSubresouceData, &g_IndexBuffer));

	D3D11_BUFFER_DESC constantBufferDesc;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.ByteWidth = sizeof(dx::XMMATRIX);
	constantBufferDesc.MiscFlags = 0;

	ThrowIfFailed(g_Device->CreateBuffer(&constantBufferDesc, nullptr, &g_ConstantBuffers[ConstantBuffers::CB_Applcation]));
	ThrowIfFailed(g_Device->CreateBuffer(&constantBufferDesc, nullptr, &g_ConstantBuffers[ConstantBuffers::CB_Frame]));
	ThrowIfFailed(g_Device->CreateBuffer(&constantBufferDesc, nullptr, &g_ConstantBuffers[ConstantBuffers::CB_Object]));

	wrl::ComPtr<ID3DBlob> vertexBlob = LoadShader(L"../Shaders/TestShader.hlsl", "VsMain", "vs_5_0");
	wrl::ComPtr<ID3DBlob> pixelBlob = LoadShader(L"../Shaders/TestShader.hlsl", "PsMain", "ps_5_0");

	ThrowIfFailed(g_Device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &g_VertexShader));
	ThrowIfFailed(g_Device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &g_PixelShader));

	D3D11_INPUT_ELEMENT_DESC inputElements[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	ThrowIfFailed(g_Device->CreateInputLayout(inputElements, _countof(inputElements), vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &g_InputLayout));

	RECT clientRect = {};
	::GetClientRect(g_WindowHandle, &clientRect);

	float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
	float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);

	g_ProjectionMatrix = dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(45.0f), clientWidth / clientHeight, 0.1f, 100.0f);

	g_DeviceContext->UpdateSubresource(g_ConstantBuffers[ConstantBuffers::CB_Applcation].Get(), 0, nullptr, &g_ProjectionMatrix, 0, 0);
}

void Init()
{
	// Init swapchain and device and device context
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = 0;
	swapChainDesc.BufferDesc.Height = 0;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = g_WindowHandle;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.Flags = 0;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	ThrowIfFailed(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags,
		nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc, &g_SwapChain, &g_Device, nullptr, &g_DeviceContext));


	// Create render target view for swapchain backbuffer.
	wrl::ComPtr<ID3D11Texture2D> backBuffer;
	ThrowIfFailed(g_SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));

	ThrowIfFailed(g_Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &g_RenderTargetView));

	// Create depth stencil buffer and view.
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc = {};
	depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilBufferDesc.ArraySize = 1;
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.SampleDesc.Quality = 0;
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilBufferDesc.CPUAccessFlags = 0;
	depthStencilBufferDesc.Width = g_ClientWidth;
	depthStencilBufferDesc.Height = g_ClientHeight;

	ThrowIfFailed(g_Device->CreateTexture2D(&depthStencilBufferDesc, nullptr, &g_DepthStencilBuffer));

	ThrowIfFailed(g_Device->CreateDepthStencilView(g_DepthStencilBuffer.Get(), nullptr, &g_DepthStencilView));

	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc = {};
	depthStencilStateDesc.DepthEnable = TRUE;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilStateDesc.StencilEnable = FALSE;

	ThrowIfFailed(g_Device->CreateDepthStencilState(&depthStencilStateDesc, &g_DepthStencilState));

	D3D11_RASTERIZER_DESC rasterizerStateDesc = {};
	rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
	rasterizerStateDesc.DepthBias = 0;
	rasterizerStateDesc.DepthBiasClamp = 0.0f;
	rasterizerStateDesc.DepthClipEnable = TRUE;
	rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerStateDesc.FrontCounterClockwise = FALSE;
	rasterizerStateDesc.ScissorEnable = FALSE;
	rasterizerStateDesc.SlopeScaledDepthBias = 0.0f;

	ThrowIfFailed(g_Device->CreateRasterizerState(&rasterizerStateDesc, &g_RasterizerState));

	g_Viewport = {};
	g_Viewport.TopLeftX = 0.0f;
	g_Viewport.TopLeftY = 0.0f;
	g_Viewport.Width = g_ClientWidth;
	g_Viewport.Height = g_ClientHeight;
	g_Viewport.MinDepth = 0.0f;
	g_Viewport.MaxDepth = 1.0f;

	// Set states to pipelnie
	g_DeviceContext->RSSetViewports(1u, &g_Viewport);
	g_DeviceContext->OMSetRenderTargets(1u, g_RenderTargetView.GetAddressOf(), g_DepthStencilView.Get());
}

void Update()
{
	dx::XMVECTOR eyePosition = dx::XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);
	dx::XMVECTOR focusPosition = dx::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	dx::XMVECTOR upDirection = dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	g_ViewMatrix = dx::XMMatrixLookAtLH(eyePosition, focusPosition, upDirection);
	
	g_DeviceContext->UpdateSubresource(g_ConstantBuffers[CB_Frame].Get(), 0, nullptr, &g_ViewMatrix, 0, 0);

	static float angle = 0.0f;
	angle += 90.0f * g_DeltaTime;

	dx::XMVECTOR rotationAxis = dx::XMVectorSet(0.0f, 1.0f, 1.0f, 0.0f);

	g_ModelMatrix = dx::XMMatrixRotationAxis(rotationAxis, angle);
	g_DeviceContext->UpdateSubresource(g_ConstantBuffers[CB_Object].Get(), 0, nullptr, &g_ModelMatrix, 0, 0);
}

void Render()
{
	g_DeviceContext->OMSetRenderTargets(1u, g_RenderTargetView.GetAddressOf(), g_DepthStencilView.Get());

	float clearColor[] = { 0.7f, 0.5f, 0.0f, 1.0f };
	g_DeviceContext->ClearRenderTargetView(g_RenderTargetView.Get(), clearColor);
	g_DeviceContext->ClearDepthStencilView(g_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0.0f);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	g_DeviceContext->IASetVertexBuffers(0, 1, g_VertexBuffer.GetAddressOf(), &stride, &offset);
	g_DeviceContext->IASetInputLayout(g_InputLayout.Get());
	g_DeviceContext->IASetIndexBuffer(g_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_DeviceContext->VSSetShader(g_VertexShader.Get(), nullptr, 0);
	g_DeviceContext->VSSetConstantBuffers(0, 3, g_ConstantBuffers[0].GetAddressOf());
	g_DeviceContext->PSSetShader(g_PixelShader.Get(), nullptr, 0);

	g_DeviceContext->OMSetDepthStencilState(g_DepthStencilState.Get(), 1);
	g_DeviceContext->RSSetState(g_RasterizerState.Get());

	g_DeviceContext->DrawIndexed(_countof(g_CubeIndices), 0, 0);
	g_SwapChain->Present(1, 0);
}

int Run()
{
	MSG message = {};

	while (message.message != WM_QUIT)
	{
		if (::PeekMessageW(&message, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessageW(&message);
		}

		g_CurrentFrameTime = g_Clock.now();
		g_DeltaTime = (g_CurrentFrameTime - g_PreviousFrameTime).count() * 1e-9;
		g_PreviousFrameTime = g_CurrentFrameTime;

		Update();
		Render();
	}

	return static_cast<int>(message.wParam);
}

int wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int)
{
	CreateAppWindow(instance);

	Init();
	LoadContent();

	ShowWindow(g_WindowHandle, SW_SHOWDEFAULT);
	return Run();
}