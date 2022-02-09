#pragma once

#include "Pch.hpp"

namespace rad
{
	class Texture
	{
	public:
		void Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& filePath, bool srgbTexture = true);
		void Bind(ID3D11DeviceContext* deviceContext, int slot = 0);

		// If a particular texture is not present, loads a default one
		static Texture DefaultTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	public:
		int m_TexWidth{ 0 };
		int m_TexHeight{ 0 };
		int m_TexChannels{ 0 };

		wrl::ComPtr<ID3D11Texture2D> m_Texture;
		wrl::ComPtr<ID3D11ShaderResourceView> m_TextureView;
	};
}

