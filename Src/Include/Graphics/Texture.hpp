#include "Pch.hpp"

class Texture
{
public:
	// WARNING : Trying to avoid using std::string however since tinyobjloader expects const char * this isnt possible as of now.
	// Problem should be fixed when I switch over to assimp / other loaders.
	void Init(const wrl::ComPtr<ID3D11Device>& device, const std::string& filePath);
	void Bind(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext, int slot = 0);

public:
	int m_TexWidth{0};
	int m_TexHeight{0};
	int m_TexChannels{0};

	wrl::ComPtr<ID3D11Texture2D> m_Texture;
	wrl::ComPtr<ID3D11ShaderResourceView> m_TextureView;
};