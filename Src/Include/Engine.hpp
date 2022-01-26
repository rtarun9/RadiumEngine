#include "Pch.hpp"

#include "Camera.hpp"

#include "Graphics/Shader.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/InputLayout.hpp"

class Engine
{
	struct Vertex
	{
		dx::XMFLOAT3 position;
		dx::XMFLOAT3 color;
	};

	std::vector<Vertex> m_CubeVertices =
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

	std::vector<uint32_t> m_CubeIndices =
	{
		0, 1, 2, 0, 2, 3,
		4, 6, 5, 4, 7, 6,
		4, 5, 1, 4, 1, 0,
		3, 2, 6, 3, 6, 7,
		1, 5, 6, 1, 6, 2,
		4, 0, 3, 4, 3, 7
	};

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

	ConstantBuffer<dx::XMMATRIX> m_ConstantBuffers[ConstantBuffers::ConstantBufferCount];

	dx::XMMATRIX m_ModelMatrix;
	dx::XMMATRIX m_ViewMatrix;
	dx::XMMATRIX m_ProjectionMatrix;

	Camera m_Camera;
};