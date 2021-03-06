#include "Pch.hpp"

#include "Graphics/API/InputLayout.hpp"

namespace rad
{
	void InputLayout::AddInputElement(const char *semanticName, DXGI_FORMAT format)
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDesc = {};
		inputElementDesc.SemanticName = semanticName;
		inputElementDesc.SemanticIndex = 0;
		inputElementDesc.Format = format;
		inputElementDesc.InputSlot = 0;
		inputElementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		inputElementDesc.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		inputElementDesc.InstanceDataStepRate = 0;

		m_InputElementDescs.push_back(std::move(inputElementDesc));
	}

	void InputLayout::Init(ID3D11Device* device, ID3DBlob* vertexBlob)
	{
		ThrowIfFailed(device->CreateInputLayout(m_InputElementDescs.data(), m_InputElementDescs.size(),
			vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &m_InputLayout));
	}

	void InputLayout::Bind(ID3D11DeviceContext* deviceContext)
	{
		deviceContext->IASetInputLayout(m_InputLayout.Get());
	}
}
