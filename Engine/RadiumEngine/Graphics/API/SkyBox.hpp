#pragma once

#include "Pch.hpp"

namespace rad
{
	class SkyBox
	{
	public:
		// To be used for CubeMaps (by providing path to images (6 for cubemap)
		void Init(ID3D11Device* device, std::wstring_view filePath, bool srgbTexture , bool hdri_texture = false);

		// To be used for equirectangular map
		void Init(ID3D11Device* device, std::wstring_view filePath);

		void Bind(ID3D11DeviceContext* deviceContext, int slot = 0);

	public:
		int m_TexWidth{ 0 };
		int m_TexHeight{ 0 };
		int m_TexChannels{ 0 };

		wrl::ComPtr<ID3D11Texture2D> m_Texture;
		wrl::ComPtr<ID3D11ShaderResourceView> m_TextureView;
	};
}

