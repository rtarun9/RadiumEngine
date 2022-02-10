#include "Graphics/API/Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace rad
{
	void Texture::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& filePath, bool srgbTexture)
	{
		unsigned char* texture = stbi_load(WStringToString(filePath).c_str(), &m_TexWidth, &m_TexHeight, &m_TexChannels, 4);
		if (!texture)
		{
			RAD_CORE_WARN("Failed to load texture with path : {0}. Using default texture instead", WStringToString(filePath));
			*this = DefaultTexture(device, deviceContext);
			return;
		}

		ASSERT(!(m_TexWidth < 0 || m_TexHeight < 0), L"Texture has invalid dimensions (width or height < 0)");

		int textureBytesPerRow = 4 * m_TexWidth;

		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = m_TexWidth;
		textureDesc.Height = m_TexHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;

		if (srgbTexture)
		{
			textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		}
		else
		{
			textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		}

		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

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

		deviceContext->GenerateMips(m_TextureView.Get());

		stbi_image_free(texture);

		//RAD_CORE_INFO("Loaded texture with path : {0}", WStringToString(filePath));
	}

	Texture Texture::DefaultTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	{
		Texture defaultTexture;
		defaultTexture.Init(device, deviceContext, L"../Assets/Textures/MissingTexture.png");

		return defaultTexture;
	}

	void Texture::Bind(ID3D11DeviceContext* deviceContext, int slot)
	{
		deviceContext->PSSetShaderResources(slot, 1, m_TextureView.GetAddressOf());
	}

}

