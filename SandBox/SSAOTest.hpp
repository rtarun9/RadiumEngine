#pragma once

#include "Pch.hpp"

#include "RadiumEngine.hpp"

namespace rad
{
	struct PerFrameData
	{
		dx::XMMATRIX viewMatrix;
		dx::XMMATRIX projectionMatrix;

		dx::XMMATRIX lightViewMatrix;
		dx::XMMATRIX lightProjectionMatrix;
	};

	struct CameraConstantBuffer
	{
		dx::XMFLOAT3 cameraPosition;
		float padding;
	};

	struct VPConstantBuffer
	{
		dx::XMMATRIX inverseTransposeVPMatrix;
	};

	struct SSAOData
	{
		// float4 due to padding reasons.
		dx::XMFLOAT4 samples[64];
		dx::XMMATRIX projectionMatrix;

	};

	class SSAOTest : public EngineBase
	{
	public:
		SSAOTest(std::wstring_view title, uint32_t width, uint32_t height);
		~SSAOTest() = default;

		void OnInit() override;
		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnDestroy() override;

		void OnKeyDown(uint32_t keycode) override;
		void OnKeyUp(uint32_t keycode) override;

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
		void GeometryPass();
		void SSAOPass();
		void BloomPass();
		void RenderPass();

		void Clear();
		void Present();

		void LogErrors();

	private:
		static constexpr uint32_t NUMBER_OF_FRAMES = 3;
		static constexpr uint32_t BLOOM_PASSES = 6;

		// To be != 1 only when Guassian blur is implemented.
		static constexpr uint32_t BLUR_PASSES = 1;

		static constexpr uint32_t SSAO_SAMPLE_KERNEL_COUNT = 64;
		static constexpr uint32_t SSAO_NOISE_TEXTURE_DIMENSION = 128;

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
		wrl::ComPtr<ID3D11DepthStencilState> m_SkyBoxDepthStencilState;

		wrl::ComPtr<ID3D11RasterizerState> m_RasterizerState;
		wrl::ComPtr<ID3D11RasterizerState> m_SkyBoxRasterizerState;

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
		RenderTarget m_BloomPassRTs[BLOOM_PASSES];
		RenderTarget m_BlurRT;

		struct GeometryPassData
		{
			RenderTarget m_PositionRT{};
			RenderTarget m_NormalRT{};
			RenderTarget m_AlbedoRT{};

			DepthStencil m_DepthStencil{};
			ShaderModule m_GeometryModule{};
		};

		GeometryPassData m_GeometryPassData{};
		RenderTarget m_SsaoRT{};
		ShaderModule m_SSAOShaderModule{};
		DepthStencil m_SSAODepthStencil{};
		
		ShaderModule m_DeferredShaderModule{};
		RenderTarget m_DeferredRT{};

		TextureSampler m_WrapSampler;
		TextureSampler m_ClampSampler;

		Camera m_Camera;

		UIManager m_UIManager;
		Log m_Log;

		DirectionalLight m_DirectionalLight;

		ConstantBuffer<PerFrameData> m_PerFrameConstantBuffer;
		ConstantBuffer<CameraConstantBuffer> m_CameraConstantBuffer;
		ConstantBuffer<VPConstantBuffer> m_VPConstantBuffer;
		ConstantBuffer<SSAOData> m_SSAOData;

		ShaderModule m_LightShader{};

		std::vector<dx::XMFLOAT3> m_SSAONoise;

		wrl::ComPtr<ID3D11Texture2D> m_NoiseTexture{};
		wrl::ComPtr<ID3D11ShaderResourceView> m_NoiseSRV{};
	};
}
