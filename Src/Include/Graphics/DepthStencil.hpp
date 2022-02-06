#pragma once

#include "Pch.hpp"

namespace rad
{
	enum class DSType
	{
		DepthStencil,
		ShadowDepth,
		None
	};

	class DepthStencil
	{
	public:
		void Init(ID3D11Device* device, uint32_t width, uint32_t height, DSType type);

		void Clear(ID3D11DeviceContext* deviceContext, float depthValue = 1.0f, float stencilValue = 0.0f);

		// Used to create a shader resource view for the depth buffer.
		wrl::ComPtr<ID3D11ShaderResourceView> ConvertToSRV(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	public:
		wrl::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
		wrl::ComPtr<ID3D11Texture2D> m_DepthStencilBuffer;

		DSType m_DepthStencilType{ DSType::None };
	};
}
