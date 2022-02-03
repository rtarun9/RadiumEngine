#include "Graphics/Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace rad
{
	void Texture::Init(const wrl::ComPtr<ID3D11Device>& device, const std::wstring& filePath)
	{
		unsigned char* texture = stbi_load(WStringToString(filePath).c_str(), &m_TexWidth, &m_TexHeight, &m_TexChannels, 4);
		if (!texture)
		{
			ErrorMessage(L"Failed to load texture at path : " + filePath);
		}

		ASSERT(!(m_TexWidth < 0 || m_TexHeight < 0), L"Texture has invalid dimensions (width or height < 0)");

		int textureBytesPerRow = 4 * m_TexWidth;

		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = m_TexWidth;
		textureDesc.Height = m_TexHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
		textureSubresourceData.pSysMem = texture;
		textureSubresourceData.SysMemPitch = textureBytesPerRow;
		textureSubresourceData.SysMemSlicePitch = 0;

		// Create texture view
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;

		ThrowIfFailed(device->CreateTexture2D(&textureDesc, &textureSubresourceData, &m_Texture));

		ThrowIfFailed(device->CreateShaderResourceView(m_Texture.Get(), &shaderResourceViewDesc, &m_TextureView));

		stbi_image_free(texture);
	}

	Texture Texture::DefaultTexture(const wrl::ComPtr<ID3D11Device>& device)
	{
		Texture defaultTexture;
		defaultTexture.Init(device, L"../Assets/Textures/MissingTexture.png");

		return defaultTexture;
	}

	void Texture::Bind(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext, int slot)
	{
		deviceContext->PSSetShaderResources(slot, 1, m_TextureView.GetAddressOf());
	}

}

