#include "Graphics/DepthStencil.hpp"

namespace rad
{
	void DepthStencil::Init(const wrl::ComPtr<ID3D11Device>& device, uint32_t width, uint32_t height, DSType type)
	{
		// All DXGI Format's are hard coded, which should be fine for a long time unless someother formats are required.
		D3D11_TEXTURE2D_DESC depthStencilBufferDesc = {};

		depthStencilBufferDesc.ArraySize = 1;
		depthStencilBufferDesc.MipLevels = 1;
		depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilBufferDesc.SampleDesc.Count = 1;
		depthStencilBufferDesc.SampleDesc.Quality = 0;
		depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		depthStencilBufferDesc.CPUAccessFlags = 0;

		depthStencilBufferDesc.Width = width;
		depthStencilBufferDesc.Height = height;

		if (type == DSType::DepthStencil)
		{
			depthStencilBufferDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		}
		else if (type == DSType::ShadowDepth)
		{
			depthStencilBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		}

		ThrowIfFailed(device->CreateTexture2D(&depthStencilBufferDesc, nullptr, &m_DepthStencilBuffer));

		// Configure view data
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};

		if (type == DSType::DepthStencil)
		{
			depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		}
		else if (type == DSType::ShadowDepth)
		{
			depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		}

		depthStencilViewDesc.Flags = 0;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		ThrowIfFailed(device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &depthStencilViewDesc, &m_DepthStencilView));

		m_DepthStencilType = type;
	}

	void DepthStencil::Clear(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext, float depthValue, float stencilValue)
	{
		if (m_DepthStencilType == DSType::DepthStencil)
		{
			deviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depthValue, stencilValue);
		}
		else if (m_DepthStencilType == DSType::ShadowDepth)
		{
			deviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, depthValue, 1u);
		}
	}

	wrl::ComPtr<ID3D11ShaderResourceView> DepthStencil::ConvertToSRV(const wrl::ComPtr<ID3D11Device>& device, const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;

		wrl::ComPtr<ID3D11ShaderResourceView> depthTextureShaderResourceView;
		ThrowIfFailed(device->CreateShaderResourceView(m_DepthStencilBuffer.Get(), &shaderResourceViewDesc, &depthTextureShaderResourceView));
		
		return depthTextureShaderResourceView;
	}
}
