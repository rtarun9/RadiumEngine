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
		
		m_ShadowDepthMap.Init(m_Device, m_Width, m_Height, DSType::ShadowDepth);
	}

	void Engine::OnUpdate(float deltaTime)
	{
		m_Camera.Update(deltaTime);

		m_PerFrameConstantBuffer.m_Data.viewMatrix = m_Camera.GetViewMatrix();

		m_PerFrameConstantBuffer.Update(m_DeviceContext);

		m_DirectionalLight.Update(m_DeviceContext);
	}

	void Engine::OnRender()
	{
		m_UIManager.FrameBegin();

		UpdateGameObjects();
		UpdateLights();

		m_Camera.UpdateControls();

		m_InputLayout.Bind(m_DeviceContext);

		m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_DeviceContext->RSSetState(m_RasterizerState.Get());
		m_DeviceContext->RSSetViewports(1u, &m_Viewport);

		// Setup for shadow pass
		ShadowRenderPass();
		
		// Setup for Main render pass (for objects)
		m_DeviceContext->OMSetRenderTargets(1u, m_RenderTargetView.GetAddressOf(), m_DepthStencil.m_DepthStencilView.Get());
		m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState.Get(), 1);
	
		Clear();

		m_Sampler.Bind(m_DeviceContext);

		// Bind constant buffers
		m_DirectionalLight.m_LightConstantBuffer.BindPS(m_DeviceContext);

		GetShaderModule(L"DefaultShader")->Bind(m_DeviceContext);
		m_PerFrameConstantBuffer.BindVS(m_DeviceContext, ConstantBuffers::CB_Frame);
		RenderGameObjects();

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
		m_Sampler.Init(m_Device, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP);
		m_PerFrameConstantBuffer.Init(m_Device);

		m_Shaders[L"DefaultShader"].vertexShader.Init(m_Device, L"../Shaders/BlinnVertex.hlsl", L"VsMain");
		m_Shaders[L"DefaultShader"].pixelShader.Init(m_Device, L"../Shaders/BlinnPixel.hlsl", L"PsMain");

		m_Shaders[L"ShadowShader"].vertexShader.Init(m_Device, L"../Shaders/ShadowMapVertex.hlsl", L"VsMain");
		m_Shaders[L"ShadowShader"].pixelShader.Init(m_Device, L"../Shaders/ShadowMapPixel.hlsl", L"PsMain");
		
		m_InputLayout.AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
		m_InputLayout.AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
		m_InputLayout.AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);

		m_InputLayout.Init(m_Device, m_Shaders[L"DefaultShader"]);
		m_InputLayout.Init(m_Device, m_Shaders[L"ShadowShader"]);

		m_PerFrameConstantBuffer.m_Data.projectionMatrix = dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(45.0f), m_AspectRatio, 0.1f, 1000.0f);

		m_UIManager.Init(m_Device, m_DeviceContext);

		m_DirectionalLight.Init(m_Device);

		m_GameObjects[L"Sponza"].Init(m_Device, L"../Assets/Models/Sponza/glTF/Sponza.gltf");
		m_GameObjects[L"Sponza"].m_Transform.scale = dx::XMFLOAT3(0.1f, 0.1f, 0.1f);
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

		D3D11_BLEND_DESC blendDesc = {};
		D3D11_RENDER_TARGET_BLEND_DESC& renderTargetBlendDesc = blendDesc.RenderTarget[0];

		renderTargetBlendDesc.BlendEnable = TRUE;
		renderTargetBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		renderTargetBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

		renderTargetBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
		
		renderTargetBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
		renderTargetBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
		renderTargetBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;

		renderTargetBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0] = renderTargetBlendDesc;

		ThrowIfFailed(m_Device->CreateBlendState(&blendDesc, &m_BlendState));
	
		m_Viewport = {};
		m_Viewport.TopLeftX = 0.0f;
		m_Viewport.TopLeftY = 0.0f;
		m_Viewport.Width = m_Width;
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
			if (ImGui::TreeNode(WStringToString(gameObjects.first).c_str()))
			{
				// NOTE : scaling on Y not working as expected.
				ImGui::SliderFloat3("Translate", &gameObjects.second.m_Transform.translation.x, -10.0f, 10.0f);
				ImGui::SliderFloat3("Rotate", &gameObjects.second.m_Transform.rotation.x, -180.0f, 180.0f);
				ImGui::SliderFloat3("Scale", &gameObjects.second.m_Transform.scale.x, 0.1f, 50.0f);
				ImGui::TreePop();
			}

			gameObjects.second.UpdateTransformComponent(m_DeviceContext);
		}

		if (ImGui::Button("Add cube"))
		{
			std::wstring objectName = L"Cube " + std::to_wstring(m_GameObjects.size());
			Model cubeModel = Model::CubeModel(m_Device);

			m_GameObjects.insert({ objectName, cubeModel});
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

	void Engine::RenderGameObjects()
	{
		for (auto& gameObject : m_GameObjects)
		{
			auto& object = gameObject.second;

			object.m_PerObjectConstantBuffer.BindVS(m_DeviceContext, ConstantBuffers::CB_Object);

			object.Draw(m_DeviceContext);
		}

	}

	void Engine::ShadowRenderPass()
	{
		GetShaderModule(L"ShadowShader")->Bind(m_DeviceContext);
		m_DirectionalLight.UpdatePerFrameData(m_DeviceContext);
		m_DirectionalLight.m_PerFrameConstantBuffer.BindVS(m_DeviceContext, ConstantBuffers::CB_Frame);

		m_ShadowDepthMap.Clear(m_DeviceContext);

		m_DeviceContext->OMSetRenderTargets(0u, nullptr, m_ShadowDepthMap.m_DepthStencilView.Get());
		m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState.Get(), 1);

		RenderGameObjects();
	}

	void Engine::Clear()
	{
		// WARNING : This static should technically not be a problem, but find a alternative method for this regardless.
		// Current problem : Clear Color keeps getting set to 0, 0, 0, 0 as it is a local variable, but making it static prevents this.
		static float clearColor[4];

		ImGui::Begin("Clear Color");
		ImGui::ColorPicker4("Clear Color", clearColor);
		ImGui::End();

		m_DeviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), clearColor);
		m_DepthStencil.Clear(m_DeviceContext);
	}

	void Engine::Present()
	{
		m_SwapChain->Present(1, 0);
	}

	ShaderModule* Engine::GetShaderModule(const std::wstring& name)
	{
		auto it = m_Shaders.find(name);
		if (it == m_Shaders.end())
		{
			ErrorMessage(L"Could not find shader with module " + name);
		}

		return &it->second;
	}
}
