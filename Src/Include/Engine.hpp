#include "Pch.hpp"

#include "Camera.hpp"
#include "UIManager.hpp"

#include "Graphics/DepthStencil.hpp"
#include "Graphics/Mesh.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Buffer.hpp"
#include "Graphics/InputLayout.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/TextureSampler.hpp"
#include "Graphics/Lights.hpp"

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
	void UpdateGameObjects();

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

	DepthStencil m_DepthStencil;

	wrl::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;

	wrl::ComPtr<ID3D11DepthStencilState> m_DepthStencilState;
	wrl::ComPtr<ID3D11RasterizerState> m_RasterizerState;

	D3D11_VIEWPORT m_Viewport = {};

	// Application specific variables.
	std::unordered_map<std::string, ShaderModule> m_Shaders;
	std::unordered_map<std::string, Mesh> m_GameObjects;

	InputLayout m_InputLayout;

	Camera m_Camera;

	UIManager m_UIManager;

	struct PerFrameData
	{
		dx::XMMATRIX viewMatrix;
		dx::XMMATRIX projectionMatrix;
	};

	DirectionalLight m_DirectionalLight;
	PerFrameData m_PerFrameData;

	ConstantBuffer<PerFrameData> m_PerFrameConstantBuffer;
};