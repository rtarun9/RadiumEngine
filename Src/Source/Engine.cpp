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

		m_PerFrameConstantBuffer.m_Data.viewMatrix = m_Camera.GetViewMatrix();
		m_PerFrameConstantBuffer.m_Data.lightViewMatrix = m_DirectionalLight.m_PerFrameConstantBuffer.m_Data.lightViewMatrix;
		m_PerFrameConstantBuffer.m_Data.lightProjectionMatrix = m_DirectionalLight.m_PerFrameConstantBuffer.m_Data.lightProjectionMatrix;

		m_PerFrameConstantBuffer.Update(m_DeviceContext.Get());

		m_DirectionalLight.Update(m_DeviceContext.Get());

		m_OffscreenRT.Update(m_DeviceContext.Get());
		m_BloomPreFilterRT.Update(m_DeviceContext.Get());
	}

	void Engine::OnRender()
	{
		m_UIManager.FrameBegin();

		UpdateGameObjects();
		UpdateLights();

		m_UIManager.ShowMetrics();

		m_Camera.UpdateControls();

		m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_DeviceContext->RSSetState(m_RasterizerState.Get());
		m_DeviceContext->RSSetViewports(1u, &m_Viewport);

		// Setup for shadow pass
		ShadowRenderPass();
		
		// Setup for Main render pass (for objects)
		m_DeviceContext->OMSetRenderTargets(1u, m_OffscreenRT.m_RTV.GetAddressOf(), m_DepthStencil.m_DepthStencilView.Get());
		m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState.Get(), 1);
	
		Clear();

		// As shadow map is modifying viewport this is necessary,
		m_DeviceContext->RSSetViewports(1u, &m_Viewport);

		m_WrapSampler.Bind(m_DeviceContext.Get());
		m_ClampSampler.Bind(m_DeviceContext.Get(), 1);

		// Bind constant buffers
		m_DirectionalLight.m_LightConstantBuffer.BindPS(m_DeviceContext.Get());

		m_DefaultShaderModule.Bind(m_DeviceContext.Get());
		m_PerFrameConstantBuffer.BindVS(m_DeviceContext.Get(), ConstantBuffers::CB_Frame);

		wrl::ComPtr<ID3D11ShaderResourceView> depthMapShaderResourceView = m_ShadowDepthMap.ConvertToSRV(m_Device.Get(), m_DeviceContext.Get());
		m_DeviceContext->PSSetShaderResources(4, 1, depthMapShaderResourceView.GetAddressOf());

		RenderGameObjects();

		// NOTE : Setting shader resource at slot 4 does cause warnings. This workaround fixes it.
		ID3D11ShaderResourceView *nullSRVs[] =
		{
			nullptr
		};

		m_DeviceContext->PSSetShaderResources(4, 1, nullSRVs);

		// Apply Bloom
		//BloomPass();

		// Apply any more postprocessing, and render to swapchain's backbuffer.
		RenderPass();
		
		// FOR DEBUGGING ONLY : Render the SRV's onto ImGui::Image.
		m_UIManager.DisplayRenderTarget(L"Shadow Depth Buffer", depthMapShaderResourceView.Get());
		m_UIManager.DisplayRenderTarget(L"Offscreen RT", m_OffscreenRT.m_SRV.Get());
		m_UIManager.DisplayRenderTarget(L"Bloom Prefilter RT", m_BloomPreFilterRT.m_SRV.Get());

		m_UIManager.Render();

		Present();

		LogErrors();

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

		if (keycode == VK_SPACE)
		{
			m_UIManager.m_DisplayUI = false;
		}
	}

	void Engine::OnKeyUp(uint32_t keycode)
	{
		m_Camera.HandleInput(keycode, false);

		if (keycode == VK_SPACE)
		{
			m_UIManager.m_DisplayUI = true;
		}
	}

	void Engine::LoadContent()
	{
		m_UIManager.Init(m_Device.Get(), m_DeviceContext.Get());
	
		m_DefaultShaderModule.Init(m_Device.Get(), InputLayoutType::DefaultInput, L"../Shaders/TestVertex.hlsl", L"../Shaders/TestPixel.hlsl");
		
		// Setup shadow shader
		m_ShadowShaderModule.Init(m_Device.Get(), InputLayoutType::DefaultInput, L"../Shaders/ShadowMapVertex.hlsl", L"../Shaders/ShadowMapPixel.hlsl");

		// Setup Render target shader
		m_RenderTargetShaderModule.Init(m_Device.Get(), InputLayoutType::RenderTargetInput, L"../Shaders/RenderTargetVertex.hlsl", L"../Shaders/RenderTargetPixel.hlsl");
		
		// Setup shader for bloom
		m_BloomPreFilterShaderModule.Init(m_Device.Get(), InputLayoutType::RenderTargetInput, L"../Shaders/Bloom/BloomPrefilterVertex.hlsl", L"../Shaders/Bloom/BloomPrefilterPixel.hlsl");
		
		m_BloomPassShaderModule.Init(m_Device.Get(), InputLayoutType::RenderTargetInput, L"../Shaders/Bloom/BloomPassVertex.hlsl", L"../Shaders/Bloom/BloomPassPixel.hlsl");

		// setup up for the blur shaders.
		m_BlurShaderModule.Init(m_Device.Get(), InputLayoutType::RenderTargetInput, L"../Shaders/Bloom/BlurVertex.hlsl", L"../Shaders/Bloom/BlurPixel.hlsl");

		m_PostProcessShaderModule.Init(m_Device.Get(), InputLayoutType::RenderTargetInput, L"../Shaders/PostProcessVertex.hlsl", L"../Shaders/PostProcessPixel.hlsl");

		// Setup for samplers.
		m_WrapSampler.Init(m_Device.Get(), D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);
		m_ClampSampler.Init(m_Device.Get(), D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP);

		// Setup constant buffers
		m_PerFrameConstantBuffer.Init(m_Device.Get());
		m_PerFrameConstantBuffer.m_Data.projectionMatrix = dx::XMMatrixPerspectiveFovLH(dx::XMConvertToRadians(45.0f), m_AspectRatio, 0.1f, 10000.0f);

		// Setup for game objects
		m_GameObjects[L"Sponza"].Init(m_Device.Get(), L"../Assets/Models/Sponza/glTF/Sponza.gltf");
		m_GameObjects[L"Sponza"].m_Transform.scale = dx::XMFLOAT3(0.1f, 0.1f, 0.1f);

		m_DirectionalLight.Init(m_Device.Get());

		// Setup Render targets.
		m_OffscreenRT.Init(m_Device.Get(), m_Width, m_Height);

		// Setup bloom RT's
		m_BloomPreFilterRT.Init(m_Device.Get(), m_Width, m_Height);

		int previousPassWidth = m_Width;
		int previousPassHeight = m_Height;
		for (int i = 0; i < BLOOM_PASSES; i++)
		{
			previousPassWidth /= 2;
			previousPassHeight /= 2;

			m_BloomPassRTs[i].Init(m_Device.Get(), previousPassWidth, previousPassHeight);

		}

		m_BlurRT.Init(m_Device.Get(), m_Width, m_Height);

		// Setup depth maps for shadow mapping.
		m_ShadowDepthMap.Init(m_Device.Get(), DirectionalLight::SHADOW_MAP_DIMENSION, DirectionalLight::SHADOW_MAP_DIMENSION, DSType::ShadowDepth);

		m_PostProcessRT.Init(m_Device.Get(), m_Width, m_Height);

		RAD_CORE_INFO("Content loaded succesfully");
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
		m_Log.Init();

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
		m_Debug.As(&m_InfoQueue);
		m_InfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
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
		m_DepthStencil.Init(m_Device.Get(), m_Width, m_Height, DSType::DepthStencil);

		RAD_CORE_INFO("Initialized renderer core");
	}

	void Engine::UpdateGameObjects()
	{
		ImGui::Begin("Scene Graph");
		for (auto& gameObjects : m_GameObjects)
		{
			if (ImGui::TreeNode(WStringToString(gameObjects.first).c_str()))
			{
				gameObjects.second.UpdateData();
			}

			gameObjects.second.UpdateTransformComponent(m_DeviceContext.Get());
		}

		if (ImGui::Button("Add cube"))
		{
			std::wstring objectName = L"Cube " + std::to_wstring(m_GameObjects.size());
			Model cubeModel = Model::CubeModel(m_Device.Get());

			m_GameObjects.insert({ objectName, cubeModel});
		}

		ImGui::End();
	}

	void Engine::UpdateLights()
	{
		m_DirectionalLight.UpdateData();
	}

	void Engine::RenderGameObjects()
	{
		for (auto& gameObject : m_GameObjects)
		{
			auto& object = gameObject.second;

			object.m_PerObjectConstantBuffer.BindVS(m_DeviceContext.Get(), ConstantBuffers::CB_Object);

			object.Draw(m_DeviceContext.Get());
		}

	}

	void Engine::ShadowRenderPass()
	{
		m_ShadowShaderModule.Bind(m_DeviceContext.Get());
		m_DirectionalLight.m_PerFrameConstantBuffer.BindVS(m_DeviceContext.Get(), ConstantBuffers::CB_Frame);

		// NOTE : Not setting RTV Will cause warnings to emit, but they are fine since this behaviour is INTENDED.
		// The warning D3D11 WARNING: ID3D11DeviceContext::DrawIndexed: The Pixel Shader expects a Render Target View bound to slot 0, but none is bound. This is OK, as writes of an unbound Render Target View are discarded. It is also possible the developer knows the data will not be used anyway. This is only a problem if the developer actually intended to bind a Render Target View here. [ EXECUTION WARNING #3146081: DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET]
		// Can be ignored.
		m_DeviceContext->OMSetRenderTargets(0u, nullptr, m_ShadowDepthMap.m_DepthStencilView.Get());
		m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState.Get(), 1);

		D3D11_VIEWPORT shadowViewport = {};
		shadowViewport.TopLeftX = 0.0f;
		shadowViewport.TopLeftY = 0.0f;
		shadowViewport.Width = DirectionalLight::SHADOW_MAP_DIMENSION;
		shadowViewport.Height = DirectionalLight::SHADOW_MAP_DIMENSION;
		shadowViewport.MinDepth = 0.0f;
		shadowViewport.MaxDepth = 1.0f;

		m_DeviceContext->RSSetViewports(1u, &shadowViewport);

		m_ShadowDepthMap.Clear(m_DeviceContext.Get(), 0.5f, 0.0f);

		RenderGameObjects();
	}

	void Engine::BloomPass()
	{
		// Blur the offscreen RT before passing to bloom prefilter
		m_BlurShaderModule.Bind(m_DeviceContext.Get());

		m_BlurRT.Bind(m_DeviceContext.Get());

		for (int i = 0; i < BLUR_PASSES; i++)
		{
			m_DeviceContext->PSSetShaderResources(0, 1, m_OffscreenRT.m_SRV.GetAddressOf());

			m_ClampSampler.Bind(m_DeviceContext.Get());

			m_BlurRT.Draw(m_DeviceContext.Get());
		}

		// Render to prefilter
		ImGui::Begin("Bloom Control");
		ImGui::SliderFloat("Threshold", &m_BloomPreFilterRT.m_RTConstantBuffer.m_Data.value, 0.0f, 5.0f);
		ImGui::End();

		m_BloomPreFilterShaderModule.Bind(m_DeviceContext.Get());

		m_BloomPreFilterRT.Bind(m_DeviceContext.Get());

		m_DeviceContext->PSSetShaderResources(0, 1, m_BlurRT.m_SRV.GetAddressOf());
		m_ClampSampler.Bind(m_DeviceContext.Get());

		m_BloomPreFilterRT.Draw(m_DeviceContext.Get());

		// Downsample passes
		m_PostProcessShaderModule.Bind(m_DeviceContext.Get());

		// First downsample pass needs PS shader resource to be the prefilter pass, so it is done outside the loop.
		m_BloomPassRTs[0].Bind(m_DeviceContext.Get());

		m_DeviceContext->OMSetRenderTargets(1, m_BloomPassRTs[0].m_RTV.GetAddressOf(), nullptr);
		m_DeviceContext->OMGetDepthStencilState(nullptr, 0u);

		ID3D11ShaderResourceView* srvs[1] =
		{
			m_BloomPreFilterRT.m_SRV.Get(),
		};

		m_DeviceContext->PSSetShaderResources(0, 1, srvs);
		m_ClampSampler.Bind(m_DeviceContext.Get());

		m_BloomPassRTs[0].Draw(m_DeviceContext.Get());

		// Downsamping passes
		for (int i = 1; i < BLOOM_PASSES; i++)
		{
			m_BloomPassRTs[i].Bind(m_DeviceContext.Get());

			ID3D11ShaderResourceView* srvs[2] =
			{
				m_BloomPassRTs[i - 1].m_SRV.Get(),			
			};
			
			m_DeviceContext->PSSetShaderResources(0, 1, srvs);
			m_ClampSampler.Bind(m_DeviceContext.Get());
			
			m_BloomPassRTs[i].Draw(m_DeviceContext.Get());
			
			// Blur texture after downsampling.
			BlurTexture(m_BloomPassRTs[i]);
		}
		
		m_BloomPassShaderModule.Bind(m_DeviceContext.Get());

		// Upsampling passes
		for (int i = BLOOM_PASSES - 1; i >= 0; i--)
		{
			m_BloomPassRTs[i - 1].Bind(m_DeviceContext.Get());
		
			m_DeviceContext->OMSetRenderTargets(1, m_BloomPassRTs[i - 1].m_RTV.GetAddressOf(), nullptr);
			m_DeviceContext->OMGetDepthStencilState(nullptr, 0u);
		
			auto currentPassSRV = m_BloomPassRTs[i - 1].m_SRV;

			ID3D11ShaderResourceView* srvs[2] =
			{
				currentPassSRV.Get(),
				m_BloomPassRTs[i].m_SRV.Get()
		
			};
		
			m_DeviceContext->PSSetShaderResources(0, 2, srvs);
			m_ClampSampler.Bind(m_DeviceContext.Get());
		
			m_BloomPassRTs[i - 1].Draw(m_DeviceContext.Get());
		
			// Blur texture after upsampling.
			BlurTexture(m_BloomPassRTs[i - 1]);
		}
	}

	void Engine::RenderPass()
	{
		// Not sure if I should make UpdateRenderpass function for updating variables such as the exposure, just leaving it in here for now.
	
		ImGui::Begin("Render Target Control");
		ImGui::SliderFloat("Exposure", &m_OffscreenRT.m_RTConstantBuffer.m_Data.value, 0.5f, 5.0f);
		ImGui::End();
		
		m_PostProcessShaderModule.Bind(m_DeviceContext.Get());

		m_PostProcessRT.Bind(m_DeviceContext.Get());

		// Pass the offscreen RT and bloom RT. In future this will be done in another render pass (Post Process pass).
		ID3D11ShaderResourceView* srvs[] =
		{
			 m_OffscreenRT.m_SRV.Get(),
			 m_BloomPassRTs[0].m_SRV.Get()
		};

		m_DeviceContext->PSSetShaderResources(0, 2, srvs);
		m_ClampSampler.Bind(m_DeviceContext.Get());

		m_PostProcessRT.Draw(m_DeviceContext.Get());
		
		// Final pass.
		m_RenderTargetShaderModule.Bind(m_DeviceContext.Get());
		
		m_DeviceContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), nullptr);
		m_DeviceContext->OMGetDepthStencilState(nullptr, 0u);

		m_DeviceContext->PSSetShaderResources(0, 1, m_PostProcessRT.m_SRV.GetAddressOf());
		m_ClampSampler.Bind(m_DeviceContext.Get());

		m_DeviceContext->DrawIndexed(6, 0, 0);
	}

	void Engine::BlurTexture(RenderTarget& rt)
	{
		// Blur the offscreen RT before passing to bloom prefilter
		m_BlurShaderModule.Bind(m_DeviceContext.Get());

		m_BlurRT.Bind(m_DeviceContext.Get());

		m_DeviceContext->OMSetRenderTargets(1, m_BlurRT.m_RTV.GetAddressOf(), nullptr);
		m_DeviceContext->OMGetDepthStencilState(nullptr, 0u);

		for (int i = 0; i < BLUR_PASSES; i++)
		{
			m_DeviceContext->PSSetShaderResources(0, 1, rt.m_SRV.GetAddressOf());

			m_ClampSampler.Bind(m_DeviceContext.Get());

			m_BlurRT.Draw(m_DeviceContext.Get());
		}

		// Not sure if a good idea or not, but doing it regardless.
		// Will have to find a way to copy the texture rather than doing another pass. This doesnt work as of now as 
		// the scales (dimentions) are very different for each incoming rt + Blur rt.
		m_PostProcessShaderModule.Bind(m_DeviceContext.Get());
		
		// First downsample pass needs PS shader resource to be the prefilter pass, so it is done outside the loop.
		rt.Bind(m_DeviceContext.Get());
		
		m_DeviceContext->OMSetRenderTargets(1, rt.m_RTV.GetAddressOf(), nullptr);
		m_DeviceContext->OMGetDepthStencilState(nullptr, 0u);
		
		ID3D11ShaderResourceView* srvs[1] =
		{
			m_BlurRT.m_SRV.Get(),
		};
		
		m_DeviceContext->PSSetShaderResources(0, 1, srvs);
		m_ClampSampler.Bind(m_DeviceContext.Get());
		
		rt.Draw(m_DeviceContext.Get());
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
		
		m_DeviceContext->ClearRenderTargetView(m_OffscreenRT.m_RTV.Get(), clearColor);

		m_DepthStencil.Clear(m_DeviceContext.Get());
	}

	void Engine::Present()
	{
		m_SwapChain->Present(1, 0);
	}

	void Engine::LogErrors()
	{
#ifdef _DEBUG
		UINT64 messageCount = m_InfoQueue->GetNumStoredMessages();

		for (UINT64 i = 0; i < messageCount; i++)
		{
			SIZE_T messageSize = 0;
			m_InfoQueue->GetMessage(i, nullptr, &messageSize);

			D3D11_MESSAGE* message = new D3D11_MESSAGE[messageSize]; 
			ThrowIfFailed(m_InfoQueue->GetMessage(i, message, &messageSize));

			RAD_CORE_WARN(message->pDescription);
			delete message;
		}

		m_InfoQueue->ClearStoredMessages();
	}
#endif
}
