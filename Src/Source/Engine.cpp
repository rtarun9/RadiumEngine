#include "Pch.hpp"

#include "Engine.hpp"
#include "Application.hpp"

Engine::Engine(const std::wstring& title, uint32_t width, uint32_t height)
	: m_Title(title), m_Width(width), m_Height(height)
{
	m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

void Engine::OnInit()
{
	RECT clientRect = {};
	::GetWindowRect(Application::GetWindowHandle(), &clientRect);

	uint32_t clientWidth = clientRect.right - clientRect.left;
	uint32_t clientHeight = clientRect.bottom - clientRect.top;

	// Init swapchain and device and device context
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = clientWidth;
	swapChainDesc.BufferDesc.Height = clientHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = Application::GetWindowHandle();
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.Flags = 0;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	ThrowIfFailed(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags,
		nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc, &m_SwapChain, &m_Device, nullptr, &m_DeviceContext));


	// Create render target view for swapchain backbuffer.
	wrl::ComPtr<ID3D11Texture2D> backBuffer;
	ThrowIfFailed(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));

	ThrowIfFailed(m_Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_RenderTargetView));

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
	depthStencilBufferDesc.Width = clientWidth;
	depthStencilBufferDesc.Height = clientHeight;

	ThrowIfFailed(m_Device->CreateTexture2D(&depthStencilBufferDesc, nullptr, &m_DepthStencilBuffer));

	ThrowIfFailed(m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), nullptr, &m_DepthStencilView));

	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc = {};
	depthStencilStateDesc.DepthEnable = TRUE;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilStateDesc.StencilEnable = FALSE;

	ThrowIfFailed(m_Device->CreateDepthStencilState(&depthStencilStateDesc, &m_DepthStencilState));

	D3D11_RASTERIZER_DESC rasterizerStateDesc = {};
	rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
	rasterizerStateDesc.DepthBias = 0;
	rasterizerStateDesc.DepthBiasClamp = 0.0f;
	rasterizerStateDesc.DepthClipEnable = TRUE;
	rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerStateDesc.FrontCounterClockwise = FALSE;
	rasterizerStateDesc.ScissorEnable = FALSE;
	rasterizerStateDesc.SlopeScaledDepthBias = 0.0f;

	ThrowIfFailed(m_Device->CreateRasterizerState(&rasterizerStateDesc, &m_RasterizerState));

	m_Viewport = {};
	m_Viewport.TopLeftX = 0.0f;
	m_Viewport.TopLeftY = 0.0f;
	m_Viewport.Width = clientWidth;
	m_Viewport.Height = clientHeight;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	LoadContent();
}

void Engine::OnUpdate(float deltaTime)
{
	dx::XMVECTOR eyePosition = dx::XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);
	dx::XMVECTOR focusPosition = dx::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	dx::XMVECTOR upDirection = dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_ViewMatrix = dx::XMMatrixLookAtLH(eyePosition, focusPosition, upDirection);

	m_ConstantBuffers[ConstantBuffers::CB_Frame].Update(m_DeviceContext, m_ViewMatrix);

	static float angle = 0.0f;
	angle += deltaTime;

	dx::XMVECTOR rotationAxis = dx::XMVectorSet(0.0f, 1.0f, 1.0f, 0.0f);

	m_ModelMatrix = dx::XMMatrixRotationAxis(rotationAxis, angle);
	m_ConstantBuffers[ConstantBuffers::CB_Object].Update(m_DeviceContext, m_ModelMatrix);
}

void Engine::OnRender()
{
	m_DeviceContext->OMSetRenderTargets(1u, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());
	m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState.Get(), 1);

	float clearColor[] = { 0.4f, 0.5f, 8.0f, 1.0f };
	m_DeviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), clearColor);
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	m_VertexBuffer.Bind(m_DeviceContext);
	m_DeviceContext->IASetInputLayout(m_InputLayout.Get());
	m_IndexBuffer.Bind(m_DeviceContext);
	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_VertexShader.Bind(m_DeviceContext);
	m_ConstantBuffers[ConstantBuffers::CB_Applcation].BindVS(m_DeviceContext, ConstantBuffers::CB_Applcation);
	m_ConstantBuffers[ConstantBuffers::CB_Frame].BindVS(m_DeviceContext, ConstantBuffers::CB_Frame);
	m_ConstantBuffers[ConstantBuffers::CB_Object].BindVS(m_DeviceContext, ConstantBuffers::CB_Object);


	m_PixelShader.Bind(m_DeviceContext);

	m_DeviceContext->RSSetState(m_RasterizerState.Get());
	m_DeviceContext->RSSetViewports(1u, &m_Viewport);

	m_DeviceContext->DrawIndexed(std::size(m_CubeIndices), 0, 0);
	m_SwapChain->Present(1, 0);
}

void Engine::OnDestroy()
{

}

void Engine::OnKeyDown(uint32_t keycode)
{

}

void Engine::OnKeyUp(uint32_t keycode)
{

}

void Engine::LoadContent()
{
	m_VertexBuffer.Init(m_Device, m_CubeVertices);
	m_IndexBuffer.Init(m_Device, m_CubeIndices);

	m_ConstantBuffers[ConstantBuffers::CB_Applcation].Init(m_Device);
	m_ConstantBuffers[ConstantBuffers::CB_Frame].Init(m_Device);
	m_ConstantBuffers[ConstantBuffers::CB_Object].Init(m_Device);

	m_VertexShader.Init(m_Device, L"../Shaders/TestShader.hlsl", "VsMain");
	m_PixelShader.Init(m_Device, L"../Shaders/TestShader.hlsl", "PsMain");

	D3D11_INPUT_ELEMENT_DESC inputElements[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	ThrowIfFailed(m_Device->CreateInputLayout(inputElements, _countof(inputElements), m_VertexShader.GetBytecodeBlob()->GetBufferPointer(), m_VertexShader.GetBytecodeBlob()->GetBufferSize(), &m_InputLayout));

	RECT clientRect = {};
	::GetClientRect(Application::GetWindowHandle(), &clientRect);

	float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
	float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);

	m_ProjectionMatrix = dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(45.0f), clientWidth / clientHeight, 0.1f, 100.0f);

	m_ConstantBuffers[ConstantBuffers::CB_Applcation].Update(m_DeviceContext, m_ProjectionMatrix);
}

uint32_t Engine::GetWidth() const
{
	return m_Width;
}

uint32_t Engine::GetHeight() const
{
	return m_Height;
}

std::wstring Engine::GetTitle() const
{
	return m_Title;
}