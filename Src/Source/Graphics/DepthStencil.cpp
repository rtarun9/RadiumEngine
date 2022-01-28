#include "Graphics/DepthStencil.hpp"

void DepthStencil::Init(const wrl::ComPtr<ID3D11Device>& device, uint32_t width, uint32_t height, DSType type)
{
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc = {};

	depthStencilBufferDesc.ArraySize = 1;
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.SampleDesc.Quality = 0;
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilBufferDesc.CPUAccessFlags = 0;
	
	depthStencilBufferDesc.Width = width;
	depthStencilBufferDesc.Height = height;

	if (type == DSType::DepthStencil)
	{
		depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	}
	else if (type == DSType::ShadowDepth)
	{
		depthStencilBufferDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	}

	ThrowIfFailed(device->CreateTexture2D(&depthStencilBufferDesc, nullptr, &m_DepthStencilBuffer));

	// Configure view data
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = depthStencilBufferDesc.Format;
	depthStencilViewDesc.Flags = 0;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	ThrowIfFailed(device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &depthStencilViewDesc, &m_DepthStencilView));

	m_DepthStencilType = type;
}

void DepthStencil::Clear(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext, float depthValue, float stencilValue)
{
	deviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depthValue, stencilValue);
}

wrl::ComPtr<ID3D11Texture2D> DepthStencil::ConvertToSRV(const wrl::ComPtr<ID3D11Device>& device, const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
{
	// Creating another texture that is compatable with source, but has CPU read access.
	wrl::ComPtr<ID3D11Resource> resource;
	m_DepthStencilView->GetResource(&resource);

	wrl::ComPtr<ID3D11Texture2D> texture;
	resource.As(&texture);

	D3D11_TEXTURE2D_DESC textureDesc = {};
	texture->GetDesc(&textureDesc);
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	textureDesc.Usage = D3D11_USAGE_STAGING;
	textureDesc.BindFlags = 0;
	
	wrl::ComPtr<ID3D11Texture2D> temporaryTexture;

	ThrowIfFailed(device->CreateTexture2D(&textureDesc, nullptr, &temporaryTexture));

	deviceContext->CopyResource(temporaryTexture.Get(), texture.Get());

	return texture;
}