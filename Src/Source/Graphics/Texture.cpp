#include "Graphics/Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace rad
{
	void Texture::Init(const wrl::ComPtr<ID3D11Device>& device, const std::string& filePath)
	{
		unsigned char* texture = stbi_load(filePath.c_str(), &m_TexWidth, &m_TexHeight, &m_TexChannels, 4);
		if (!texture)
		{
			ErrorMessage(L"Failed to load texture at path : " + StringToWString(filePath));
		}

		int textureBytesPerRow = 4 * m_TexWidth;

		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = m_TexWidth;
		textureDesc.Height = m_TexHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA textureSubresourceData = {};
		textureSubresourceData.pSysMem = texture;
		textureSubresourceData.SysMemPitch = textureBytesPerRow;

		ThrowIfFailed(device->CreateTexture2D(&textureDesc, &textureSubresourceData, &m_Texture));

		ThrowIfFailed(device->CreateShaderResourceView(m_Texture.Get(), nullptr, &m_TextureView));

		stbi_image_free(texture);
	}

	void Texture::Bind(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext, int slot)
	{
		deviceContext->PSSetShaderResources(slot, 1, m_TextureView.GetAddressOf());
	}

}

