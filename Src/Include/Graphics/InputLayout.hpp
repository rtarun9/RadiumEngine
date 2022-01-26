#include "Pch.hpp"

class InputLayout
{
public:
	void AddInputElement(const char* semanticName, DXGI_FORMAT format);

	void Init(const wrl::ComPtr<ID3D11Device>& device, const wrl::ComPtr<ID3DBlob>& vertexShaderBlob);

	void Bind(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);

public:
	wrl::ComPtr<ID3D11InputLayout> m_InputLayout;

	std::vector<D3D11_INPUT_ELEMENT_DESC> m_InputElementDescs;
};