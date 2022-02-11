#include "Graphics/API/SkyBox.hpp"

#include "stb_image.h"

namespace rad
{
	void SkyBox::Init(ID3D11Device* device, std::wstring_view  filePath, bool srgbTexture, bool hdri_texture)
	{
		std::array<unsigned char*, 6> textures{};

		std::array<std::wstring, 6> paths{};

		std::array<std::thread, 6> textureThreads{};

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
				L"/px.hdr",
				L"/nx.hdr",
				L"/py.hdr",
				L"/ny.hdr",
				L"/pz.hdr",
				L"/nz.hdr"
			};
		}

		int channels = hdri_texture ? 4 : 4;

		auto loadTexture = [&](int i)
		{
			textures[i] = stbi_load(WStringToString(filePath.data() + paths[i]).c_str(), &m_TexWidth, &m_TexHeight, &m_TexChannels, channels);

			if (!textures[i])
			{
				RAD_CORE_WARN("Failed to load texture with path : {0}. Using default texture instead", WStringToString(filePath.data()));
			}
			ASSERT(!(m_TexWidth < 0 || m_TexHeight < 0), L"Texture has invalid dimensions (width or height < 0)");
			RAD_CORE_INFO("Loaded texture with path : {0}", WStringToString(filePath.data() + paths[i]));
		};

		// Basic multithreading for loading the high res skybox textures. Doing so reduces load time from 18 seconds to 8 seconds. It is still a very naive approach however and can be made much faster.
		for (int i = 0; i < 6; i++)
		{
			textureThreads[i] = std::thread(loadTexture, i);
		}

		textureThreads[0].join();

		int textureBytesPerRow = 4 * m_TexWidth;

		D3D11_TEXTURE2D_DESC textureDesc{};
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

		D3D11_SUBRESOURCE_DATA textureSubresourceData[6]{};
		// subresource 0 is ready to be done is thread's work is already done
		textureSubresourceData[0].pSysMem = std::move(textures[0]);
		textureSubresourceData[0].SysMemPitch = textureBytesPerRow;
		textureSubresourceData[0].SysMemSlicePitch = 0;

		for (int i = 1; i < paths.size(); i++)
		{
			textureThreads[i].join();
			textureSubresourceData[i].pSysMem = std::move(textures[i]);
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

		std::fill(textures.begin(), textures.end(), nullptr);
	}

	void SkyBox::Init(ID3D11Device* device, std::wstring_view filePath)
	{
		float *textures = stbi_loadf(WStringToString(filePath.data()).c_str(), &m_TexWidth, &m_TexHeight, &m_TexChannels, 4);

		if (!textures)
		{
			RAD_CORE_WARN("Failed to load texture with path : {0}. Using default texture instead", WStringToString(filePath.data()));
		}
		ASSERT(!(m_TexWidth < 0 || m_TexHeight < 0), L"Texture has invalid dimensions (width or height < 0)");
		RAD_CORE_INFO("Loaded texture with path : {0}", WStringToString(filePath.data()));

		int textureBytesPerRow = 4 * m_TexWidth;

		D3D11_TEXTURE2D_DESC textureDesc{};
		textureDesc.Width = m_TexWidth;
		textureDesc.Height = m_TexHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA textureSubresourceData{};

		textureSubresourceData.pSysMem = textures;
		textureSubresourceData.SysMemPitch = textureBytesPerRow;
		textureSubresourceData.SysMemSlicePitch = 0;

		ThrowIfFailed(device->CreateTexture2D(&textureDesc, &textureSubresourceData, &m_Texture));

		// Create texture view
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};

		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;


		ThrowIfFailed(device->CreateShaderResourceView(m_Texture.Get(), &shaderResourceViewDesc, &m_TextureView));

		stbi_image_free(textures);
	}


	void SkyBox::Bind(ID3D11DeviceContext* deviceContext, int slot)
	{
		deviceContext->PSSetShaderResources(slot, 1, m_TextureView.GetAddressOf());
	}

}

