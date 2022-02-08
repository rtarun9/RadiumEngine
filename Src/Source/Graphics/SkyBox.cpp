#include "Graphics/SkyBox.hpp"

#include "stb_image.h"

namespace rad
{
	void SkyBox::Init(ID3D11Device* device, const std::wstring& filePath, bool srgbTexture, bool hdri_texture)
	{
		std::vector<unsigned char*> textures(6);
		
		std::vector<std::wstring> paths(6);

		if (!hdri_texture)
		{
			paths =
			{
				L"/right.jpg",
				L"/left.jpg",
				L"/top.jpg",
				L"/bottom.jpg",
				L"/front.jpg",
				L"/back.jpg"
			};
		}
		else
		{
			paths =
			{
				L"/right.hdr",
				L"/left.hdr",
				L"/top.hdr",
				L"/bottom.hdr",
				L"/front.hdr",
				L"/back.hdr"
			};
		}

		int channels = hdri_texture ? 4 : 4;

		for (int i = 0; i < paths.size(); i++)
		{
			textures[i] = stbi_load(WStringToString(filePath + paths[i]).c_str(), &m_TexWidth, &m_TexHeight, &m_TexChannels, channels);
			if (!textures[i])
			{
				RAD_CORE_WARN("Failed to load texture with path : {0}. Using default texture instead", WStringToString(filePath));
			}
			ASSERT(!(m_TexWidth < 0 || m_TexHeight < 0), L"Texture has invalid dimensions (width or height < 0)");
			RAD_CORE_INFO("Loaded texture with path : {0}", WStringToString(filePath + paths[i]));
		}
	
		int textureBytesPerRow = 4 * m_TexWidth;

		if (hdri_texture)
		{
			textureBytesPerRow = 4 * m_TexWidth;
		}

		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = m_TexWidth;
		textureDesc.Height = m_TexHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 6;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

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
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA textureSubresourceData[6] = {};
		for (int i = 0; i < paths.size(); i++)
		{
			textureSubresourceData[i].pSysMem = textures[i];
			textureSubresourceData[i].SysMemPitch = textureBytesPerRow;
			textureSubresourceData[i].SysMemSlicePitch = 0;
		}

		// Create texture view
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		shaderResourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;

		ThrowIfFailed(device->CreateTexture2D(&textureDesc, textureSubresourceData, &m_Texture));

		ThrowIfFailed(device->CreateShaderResourceView(m_Texture.Get(), &shaderResourceViewDesc, &m_TextureView));

		for (int i = 0; i < paths.size(); i++)
		{
			stbi_image_free(textures[i]);
		}
	}

	void SkyBox::Bind(ID3D11DeviceContext* deviceContext, int slot)
	{
		deviceContext->PSSetShaderResources(slot, 1, m_TextureView.GetAddressOf());
	}

}

