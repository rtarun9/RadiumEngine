#include "Pch.hpp"

#include "Graphics/Shader.hpp"

namespace rad
{
	class InputLayout
	{
	public:
		void AddInputElement(const char* semanticName, DXGI_FORMAT format);

		void Init(const wrl::ComPtr<ID3D11Device>& device, ShaderModule& shaderModule);

		void Bind(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);

	public:
		wrl::ComPtr<ID3D11InputLayout> m_InputLayout;

		std::vector<D3D11_INPUT_ELEMENT_DESC> m_InputElementDescs;
	};
}
