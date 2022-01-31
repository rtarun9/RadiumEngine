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
		void Init(const wrl::ComPtr<ID3D11Device>& device, uint32_t width, uint32_t height, DSType type);

		void Clear(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext, float depthValue = 1.0f, float stencilValue = 0.0f);

		wrl::ComPtr<ID3D11Texture2D> ConvertToSRV(const wrl::ComPtr<ID3D11Device>& device, const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);

	public:
		wrl::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
		wrl::ComPtr<ID3D11Texture2D> m_DepthStencilBuffer;

		DSType m_DepthStencilType{ DSType::None };
	};
}
