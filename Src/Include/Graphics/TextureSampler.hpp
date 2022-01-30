#include "Pch.hpp"

namespace rad
{
	class TextureSampler
	{
	public:
		void Init(const wrl::ComPtr<ID3D11Device>& device, D3D11_FILTER filter = D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_MODE addressMode = D3D11_TEXTURE_ADDRESS_CLAMP);
		void Bind(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext, int slot = 0);

	public:
		wrl::ComPtr<ID3D11SamplerState> m_SamplerState;
	};
}
