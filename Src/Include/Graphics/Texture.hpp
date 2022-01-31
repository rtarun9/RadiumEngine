#pragma once

#include "Pch.hpp"

namespace rad
{
	class Texture
	{
	public:
		void Init(const wrl::ComPtr<ID3D11Device>& device, const std::wstring& filePath);
		void Bind(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext, int slot = 0);

		// If a particular texture is not present, loads a default one
		static Texture DefaultTexture(const wrl::ComPtr<ID3D11Device>& device);

	public:
		int m_TexWidth{ 0 };
		int m_TexHeight{ 0 };
		int m_TexChannels{ 0 };

		wrl::ComPtr<ID3D11Texture2D> m_Texture;
		wrl::ComPtr<ID3D11ShaderResourceView> m_TextureView;
	};
}

