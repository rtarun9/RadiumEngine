#include "Pch.hpp"

#include "Engine.hpp"
#include "Application.hpp"

namespace rad
{

	Engine::Engine(const std::wstring& title, uint32_t width, uint32_t height)
		: m_Title(title), m_Width(width), m_Height(height)
	{
		m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
	}

	void Engine::OnInit()
	{
		InitRendererCore();

		LoadContent();
	}

	void Engine::OnUpdate(float deltaTime)
	{
		m_Camera.Update(deltaTime);

		m_PerFrameData.viewMatrix = m_Camera.GetViewMatrix();

		m_PerFrameConstantBuffer.Update(m_DeviceContext, m_PerFrameData);

		m_DirectionalLight.Update(m_DeviceContext);
	}

	void Engine::OnRender()
	{
		m_UIManager.FrameBegin();

		UpdateGameObjects();
		UpdateLights();

		m_DeviceContext->OMSetRenderTargets(1u, m_RenderTargetView.GetAddressOf(), m_DepthStencil.m_DepthStencilView.Get());
		m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState.Get(), 1);

		float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		Clear(clearColor);

		m_InputLayout.Bind(m_DeviceContext);

		m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_DeviceContext->RSSetState(m_RasterizerState.Get());
		m_DeviceContext->RSSetViewports(1u, &m_Viewport);

		// Bind constant buffers
		m_DirectionalLight.m_LightConstantBuffer.BindPS(m_DeviceContext);
		m_PerFrameConstantBuffer.BindVS(m_DeviceContext, ConstantBuffers::CB_Frame);

		GetShaderModule("DefaultShader")->Bind(m_DeviceContext);
		
		for (auto& gameObject : m_GameObjects)
		{
			auto& object = gameObject.second;
		
			object.m_VertexBuffer.Bind(m_DeviceContext);
		
		
			object.m_TransformConstantBuffer.BindVS(m_DeviceContext, ConstantBuffers::CB_Object);
		
			object.Draw(m_DeviceContext);
		}

		m_TestModel.Draw(m_DeviceContext);

		m_UIManager.Render();

		Present();
	}

	void Engine::OnDestroy()
	{
		m_DeviceContext->ClearState();
		m_DeviceContext->Flush();

		m_UIManager.Close();
	}

	void Engine::OnKeyDown(uint32_t keycode)
	{
		m_Camera.HandleInput(keycode, true);
	}

	void Engine::OnKeyUp(uint32_t keycode)
	{
		m_Camera.HandleInput(keycode, false);
	}

	void Engine::LoadContent()
	{
		m_GameObjects["Cube"].Init(m_Device, "../Assets/Models/Cube/cube.obj");
		m_GameObjects["Floor"].Init(m_Device, "../Assets/Models/Cube/cube.obj");

		m_GameObjects["Floor"].m_Transform.translation = dx::XMFLOAT3(0.0f, -6.0f, 0.0f);
		m_GameObjects["Floor"].m_Transform.scale = dx::XMFLOAT3(15.0f, 1.0f, 15.0f);

		m_PerFrameConstantBuffer.Init(m_Device);

		m_Shaders["DefaultShader"].vertexShader.Init(m_Device, L"../Shaders/BlinnVertex.hlsl", "VsMain");
		m_Shaders["DefaultShader"].pixelShader.Init(m_Device, L"../Shaders/BlinnPixel.hlsl", "PsMain");

		m_InputLayout.AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		m_InputLayout.AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
		m_InputLayout.AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

		m_InputLayout.Init(m_Device, m_Shaders["DefaultShader"]);

		m_PerFrameData.projectionMatrix = dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(45.0f), m_AspectRatio, 0.1f, 1000.0f);

		m_UIManager.Init(m_Device, m_DeviceContext);

		m_DirectionalLight.m_LightData.ambientStrength = 0.1f;
		m_DirectionalLight.m_LightData.lightColor = dx::XMFLOAT3(1.0f, 1.0f, 1.0f);
		m_DirectionalLight.m_LightData.lightDirection = dx::XMFLOAT4(0.0f, -10.0f, 0.0f, 0.0f);

		m_DirectionalLight.Init(m_Device);

		m_TestModel.Init(m_Device, "../Assets/Models/Cube/cube.obj");
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

	void Engine::InitRendererCore()
	{
		RECT clientRect = {};
		::GetWindowRect(Application::GetWindowHandle(), &clientRect);

		m_ClientWidth = clientRect.right - clientRect.left;
		m_ClientHeight = clientRect.bottom - clientRect.top;

		// Init swapchain and device and device context
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = 2;
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
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

#ifdef _DEBUG
		ThrowIfFailed(m_Device->QueryInterface(__uuidof(ID3D11Debug), &m_Debug));
#endif

		// Create render target view for swapchain backbuffer.
		wrl::ComPtr<ID3D11Texture2D> backBuffer;
		ThrowIfFailed(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));

		ThrowIfFailed(m_Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_RenderTargetView));

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
		m_Viewport.Width = m_Width;;
		m_Viewport.Height = m_Height;
		m_Viewport.MinDepth = 0.0f;
		m_Viewport.MaxDepth = 1.0f;

		// Create depth stencil buffer and view.
		m_DepthStencil.Init(m_Device, m_Width, m_Height, DSType::DepthStencil);
	}

	void Engine::UpdateGameObjects()
	{
		ImGui::Begin("Scene Graph");
		for (auto& gameObjects : m_GameObjects)
		{
			if (ImGui::TreeNode(gameObjects.first.c_str()))
			{
				// NOTE : scaling on Y not working as expected.
				ImGui::SliderFloat3("Translate", &gameObjects.second.m_Transform.translation.x, -10.0f, 10.0f);
				ImGui::SliderFloat3("Rotate", &gameObjects.second.m_Transform.rotation.x, -180.0f, 180.0f);
				ImGui::SliderFloat3("Scale", &gameObjects.second.m_Transform.scale.x, 0.1f, 50.0f);
				ImGui::ColorEdit3("Color", &gameObjects.second.m_PerObjectData.color.x);
				ImGui::TreePop();
			}

			gameObjects.second.UpdateTransformComponent(m_DeviceContext);
		}

		if (ImGui::Button("Add cube"))
		{
			std::string objectName = "Cube " + std::to_string(m_GameObjects.size());
			Mesh mesh;

			// Obvious problem here : Same mesh being loaded again and again. Not fixing it as of now since only used for small scale testing of shadow mapping.
			mesh.Init(m_Device, "../Assets/Models/Cube/cube.obj");
			m_GameObjects.insert({ objectName, mesh });
		}

		ImGui::End();
	}

	void Engine::UpdateLights()
	{
		ImGui::Begin("Light Properties");
		if (ImGui::TreeNode("Directional Light"))
		{
			ImGui::SliderFloat("Ambient Strength", &m_DirectionalLight.m_LightData.ambientStrength, 0.0f, 10.0f);
			ImGui::ColorPicker3("Color", &m_DirectionalLight.m_LightData.lightColor.x);
			ImGui::SliderFloat3("Direction", &m_DirectionalLight.m_LightData.lightDirection.x, -10.0f, 10.0f);
			ImGui::TreePop();
		}
		ImGui::End();
	}

	void Engine::Clear(float clearColor[])
	{
		m_DeviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), clearColor);
		m_DepthStencil.Clear(m_DeviceContext);
	}

	void Engine::Present()
	{
		m_SwapChain->Present(1, 0);
	}

	ShaderModule* Engine::GetShaderModule(const std::string& name)
	{
		auto it = m_Shaders.find(name);
		if (it == m_Shaders.end())
		{
			ErrorMessage(L"Could not find shader with module " + StringToWString(name));
		}

		return &it->second;
	}

	Mesh* Engine::GetMesh(const std::string& name)
	{
		auto it = m_GameObjects.find(name);
		if (it == m_GameObjects.end())
		{
			ErrorMessage(L"Could not find mesh with module " + StringToWString(name));
		}

		return &it->second;
	}
}
