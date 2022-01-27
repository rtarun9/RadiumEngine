#include "Pch.hpp"

#include "Camera.hpp"
#include "Mesh.hpp"
#include "UIManager.hpp"

#include "Graphics/Shader.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/InputLayout.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/TextureSampler.hpp"


class Engine
{
public:
	Engine(const std::wstring& title, uint32_t width, uint32_t height);
	~Engine();

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
	static constexpr uint32_t NUMBER_OF_FRAMES = 3;

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

	wrl::ComPtr<ID3D11Texture2D> m_DepthStencilBuffer;
	wrl::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
	wrl::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;

	wrl::ComPtr<ID3D11DepthStencilState> m_DepthStencilState;
	wrl::ComPtr<ID3D11RasterizerState> m_RasterizerState;

	D3D11_VIEWPORT m_Viewport = {};

	// Application specific variables.
	InputLayout m_InputLayout;
	VertexBuffer<Vertex> m_VertexBuffer;
	IndexBuffer m_IndexBuffer;

	VertexShader m_VertexShader;
	PixelShader m_PixelShader;

	std::unordered_map<std::string, Mesh> m_GameObjects;
	
	TextureSampler m_Sampler;
	Texture m_WoodTexture;

	ConstantBuffer<dx::XMMATRIX> m_ConstantBuffers[ConstantBuffers::ConstantBufferCount];

	dx::XMMATRIX m_ViewMatrix;
	dx::XMMATRIX m_ProjectionMatrix;

	Camera m_Camera;

	UIManager m_UIManager;
};