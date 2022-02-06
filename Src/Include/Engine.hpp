#pragma once

#include "Pch.hpp"

#include "Camera.hpp"
#include "UIManager.hpp"

#include "Graphics/DepthStencil.hpp"
#include "Graphics/Mesh.hpp"
#include "Graphics/Model.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/InputLayout.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/TextureSampler.hpp"
#include "Graphics/Lights.hpp"
#include "Graphics/RenderTarget.hpp"

namespace rad
{
	struct PerFrameData
	{
		dx::XMMATRIX viewMatrix;
		dx::XMMATRIX projectionMatrix;

		dx::XMMATRIX lightViewMatrix;
		dx::XMMATRIX lightProjectionMatrix;
	};

	class Engine
	{
	public:
		Engine(const std::wstring& title, uint32_t width, uint32_t height);
		~Engine() = default;

		void OnInit();
		void OnUpdate(float deltaTime);
		void OnRender();
		void OnDestroy();

		void OnKeyDown(uint32_t keycode);
		void OnKeyUp(uint32_t keycode);

		void LoadContent();

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		std::wstring GetTitle() const;

	private:
		void InitRendererCore();

		void UpdateGameObjects();
		void UpdateLights();

		void RenderGameObjects();
		void ShadowRenderPass();
		void BloomPass();
		void RenderPass();

		void BlurTexture(RenderTarget& rt);
		void Clear();
		void Present();

		void LogErrors();

	private:
		static constexpr uint32_t NUMBER_OF_FRAMES = 3;
		static constexpr uint32_t BLOOM_PASSES = 7;

		// To be != 1 only when Guassian blur is implemented.
		static constexpr uint32_t BLUR_PASSES = 1;

		// Viewport and window variables.
		uint32_t m_Width;
		uint32_t m_Height;
		std::wstring m_Title;

		float m_AspectRatio;

		float m_ClientWidth;
		float m_ClientHeight;

		// D3D11 Objects
		wrl::ComPtr<ID3D11Device> m_Device;
		wrl::ComPtr<ID3D11DeviceContext> m_DeviceContext;
		wrl::ComPtr<IDXGISwapChain> m_SwapChain;

		wrl::ComPtr<ID3D11Debug> m_Debug;
		wrl::ComPtr<ID3D11InfoQueue> m_InfoQueue;

		DepthStencil m_DepthStencil;
		DepthStencil m_ShadowDepthMap;

		wrl::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;

		wrl::ComPtr<ID3D11DepthStencilState> m_DepthStencilState;
		wrl::ComPtr<ID3D11RasterizerState> m_RasterizerState;
		wrl::ComPtr<ID3D11BlendState> m_BlendState;

		D3D11_VIEWPORT m_Viewport = {};

		// Application specific variables.
		ShaderModule m_DefaultShaderModule{};
		ShaderModule m_ShadowShaderModule{};
		ShaderModule m_RenderTargetShaderModule{};
		ShaderModule m_BloomPreFilterShaderModule{};
		ShaderModule m_BloomPassShaderModule{};
		ShaderModule m_BlurShaderModule{};
		ShaderModule m_PostProcessShaderModule{};

		std::unordered_map<std::wstring, Model> m_GameObjects;
		
		RenderTarget m_PostProcessRT;

		RenderTarget m_OffscreenRT;
		RenderTarget m_BloomPreFilterRT;

		RenderTarget m_BlurRT;

		RenderTarget m_BloomPassRTs[BLOOM_PASSES];

		TextureSampler m_WrapSampler;
		TextureSampler m_ClampSampler;

		Camera m_Camera;

		UIManager m_UIManager;
		Log m_Log;

		DirectionalLight m_DirectionalLight;
	
		ConstantBuffer<PerFrameData> m_PerFrameConstantBuffer;
	};
}
