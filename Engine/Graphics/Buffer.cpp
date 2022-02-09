#include "Graphics/Buffer.hpp"

namespace rad
{
	void IndexBuffer::Init(ID3D11Device* device, const std::vector<uint32_t>& indices)
	{
		D3D11_BUFFER_DESC indexBufferDesc = {};
		indexBufferDesc.ByteWidth = std::size(indices) * sizeof(uint32_t);
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubresourceData = {};
		indexSubresourceData.pSysMem = indices.data();

		ThrowIfFailed(device->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &m_Buffer));

		m_Size = indices.size();
	}

	void IndexBuffer::Bind(ID3D11DeviceContext *deviceContext)
	{
		deviceContext->IASetIndexBuffer(m_Buffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
	}
}
