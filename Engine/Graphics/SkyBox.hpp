#pragma once

#include "Pch.hpp"

namespace rad
{
	class SkyBox
	{
	public:
		void Init(ID3D11Device* device, const std::wstring& filePath, bool srgbTexture = true, bool hdri_texture = false);
		void Bind(ID3D11DeviceContext* deviceContext, int slot = 0);

	public:
		int m_TexWidth{ 0 };
		int m_TexHeight{ 0 };
		int m_TexChannels{ 0 };

		wrl::ComPtr<ID3D11Texture2D> m_Texture;
		wrl::ComPtr<ID3D11ShaderResourceView> m_TextureView;
	};
}

