#include "Pch.hpp"

enum ConstantBuffers
{
	CB_Applcation,
	CB_Frame,
	CB_Object,
	ConstantBufferCount
};

class Buffer
{
public:
	[[nodiscard]]
	wrl::ComPtr<ID3D11Buffer> GetBuffer()
	{
		return m_Buffer;
	}
	
protected:
	wrl::ComPtr<ID3D11Buffer> m_Buffer;
};

template <typename T>
class VertexBuffer : public Buffer
{
public:
	void Init(wrl::ComPtr<ID3D11Device> device, const std::vector<T>& vertices)
	{
		m_Stride = sizeof(T);
		m_Offset = 0;

		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.ByteWidth = std::size(vertices) * sizeof(T);
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubresourceData = {};
		vertexSubresourceData.pSysMem = vertices.data();

		ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &m_Buffer));
	}

	void Bind(wrl::ComPtr<ID3D11DeviceContext> deviceContext)
	{
		deviceContext->IASetVertexBuffers(0u, 1u, m_Buffer.GetAddressOf(), &m_Stride, &m_Offset);
	}

public:
	UINT m_Stride;
	UINT m_Offset;
};

class IndexBuffer : public Buffer
{
public:
	void Init(wrl::ComPtr<ID3D11Device> device, const std::vector<uint32_t>& indices)
	{
		D3D11_BUFFER_DESC indexBufferDesc = {};
		indexBufferDesc.ByteWidth = std::size(indices) * sizeof(uint32_t);
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubresourceData = {};
		indexSubresourceData.pSysMem = indices.data();

		ThrowIfFailed(device->CreateBuffer(&indexBufferDesc, &indexSubresourceData, &m_Buffer));
	}

	void Bind(wrl::ComPtr<ID3D11DeviceContext> deviceContext)
	{
		deviceContext->IASetIndexBuffer(m_Buffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
	}

public:
};

template <typename T>
class ConstantBuffer : public Buffer
{
public:
	void Init(wrl::ComPtr<ID3D11Device> device)
	{
		D3D11_BUFFER_DESC constantBufferDesc = {};
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		constantBufferDesc.CPUAccessFlags = 0;
		constantBufferDesc.ByteWidth = sizeof(T);
		constantBufferDesc.MiscFlags = 0;

		ThrowIfFailed(device->CreateBuffer(&constantBufferDesc, nullptr, &m_Buffer));
	}

	void Update(wrl::ComPtr<ID3D11DeviceContext> deviceContext, const T& data)
	{
		deviceContext->UpdateSubresource(m_Buffer.Get(), 0, nullptr, &data, 0, 0);
		m_Data = data;
	}

	void BindVS(wrl::ComPtr<ID3D11DeviceContext> deviceContext, UINT startSlot = 0, UINT count = 1)
	{
		deviceContext->VSSetConstantBuffers(startSlot, count, m_Buffer.GetAddressOf());
	}

	void BindPS(wrl::ComPtr<ID3D11DeviceContext> deviceContext, UINT startSlot = 0, UINT count = 1)
	{
		deviceContext->PSSetConstantBuffers(startSlot, count, m_Buffer.GetAddressOf());
	}

public:
	T m_Data;
};