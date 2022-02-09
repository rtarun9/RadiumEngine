#include "Graphics/RenderTarget.hpp"

namespace rad
{
	void RenderTarget::Init(ID3D11Device* device, int width, int height)
	{
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		ThrowIfFailed(device->CreateTexture2D(&textureDesc, nullptr, &m_Texture));

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		ThrowIfFailed(device->CreateRenderTargetView(m_Texture.Get(), &rtvDesc, &m_RTV));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		ThrowIfFailed(device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, &m_SRV));

		std::vector<RTVertex> vertices;
		vertices.push_back({ dx::XMFLOAT2(-1.0f, -1.0f), dx::XMFLOAT2(0.0f, 1.0f) });
		vertices.push_back({ dx::XMFLOAT2(-1.0f,   1.0f), dx::XMFLOAT2(0.0f, 0.0f) });
		vertices.push_back({ dx::XMFLOAT2(1.0f, 1.0f), dx::XMFLOAT2(1.0f, 0.0f) });
		vertices.push_back({ dx::XMFLOAT2(1.0f, -1.0f), dx::XMFLOAT2(1.0f, 1.0f) });

		m_Vertices.Init(device, vertices);

		std::vector<uint32_t> indices =
		{
			0, 1, 2,
			0, 2, 3
		};

		m_IndexBuffer.Init(device, indices);

		m_Viewport = {};
		m_Viewport.TopLeftX = 0;
		m_Viewport.TopLeftY = 0;
		m_Viewport.Width = width;
		m_Viewport.Height = height;
		m_Viewport.MinDepth = 0.0f;
		m_Viewport.MaxDepth = 1.0f;

		m_RTConstantBuffer.Init(device);

		m_RTConstantBuffer.m_Data.value = 1.0f;
		m_RTConstantBuffer.m_Data.padding = dx::XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	void RenderTarget::Bind(ID3D11DeviceContext* deviceContext)
	{
		deviceContext->RSSetViewports(1, &m_Viewport);

		m_Vertices.Bind(deviceContext);
		m_IndexBuffer.Bind(deviceContext);

		deviceContext->OMSetRenderTargets(1, m_RTV.GetAddressOf(), nullptr); 
		deviceContext->OMGetDepthStencilState(nullptr, 0u);

		m_RTConstantBuffer.BindPS(deviceContext);
	}

	void RenderTarget::Update(ID3D11DeviceContext* deviceContext)
	{
		m_RTConstantBuffer.Update(deviceContext);
	}

	void RenderTarget::Draw(ID3D11DeviceContext* deviceContext)
	{
		deviceContext->DrawIndexed(6, 0, 0);
	}
}