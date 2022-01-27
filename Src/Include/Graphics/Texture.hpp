#include "Pch.hpp"

class Texture
{
public:
	void Init(const wrl::ComPtr<ID3D11Device>& device, const char* filePath);
	void Bind(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext, int slot = 0);

public:
	int m_TexWidth;
	int m_TexHeight;
	int m_TexChannels;

	wrl::ComPtr<ID3D11Texture2D> m_Texture;
	wrl::ComPtr<ID3D11ShaderResourceView> m_TextureView;
};