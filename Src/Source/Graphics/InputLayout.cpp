#include "Pch.hpp"

#include "Graphics/InputLayout.hpp"

void InputLayout::AddInputElement(const char* semanticName, DXGI_FORMAT format)
{
	D3D11_INPUT_ELEMENT_DESC inputElementDesc = {};
	inputElementDesc.SemanticName = semanticName;
	inputElementDesc.SemanticIndex = 0;
	inputElementDesc.Format = format;
	inputElementDesc.InputSlot = 0;
	inputElementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	inputElementDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
	inputElementDesc.InstanceDataStepRate = 0;

	m_InputElementDescs.push_back(inputElementDesc);
}

void InputLayout::Init(const wrl::ComPtr<ID3D11Device>& device, ShaderModule& shaderModule)
{
	ThrowIfFailed(device->CreateInputLayout(m_InputElementDescs.data(), m_InputElementDescs.size(), 
		shaderModule.vertexShader.GetBytecodeBlob()->GetBufferPointer(), shaderModule.vertexShader.GetBytecodeBlob()->GetBufferSize(), &m_InputLayout));
}

void InputLayout::Bind(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
{
	deviceContext->IASetInputLayout(m_InputLayout.Get());
}