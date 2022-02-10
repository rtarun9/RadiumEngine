#include "Graphics/API/TextureSampler.hpp"

namespace rad
{
	void TextureSampler::Init(ID3D11Device* device, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode)
	{
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = filter;
		samplerDesc.AddressU = addressMode;
		samplerDesc.AddressV = addressMode;
		samplerDesc.AddressW = addressMode;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

		ThrowIfFailed(device->CreateSamplerState(&samplerDesc, &m_SamplerState));
	}

	void TextureSampler::Bind(ID3D11DeviceContext* deviceContext, int slot)
	{
		deviceContext->PSSetSamplers(slot, 1, m_SamplerState.GetAddressOf());
	}

}

