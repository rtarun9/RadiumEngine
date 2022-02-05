#pragma once

#include "Pch.hpp"

#include "InputLayout.hpp"
#include "Buffer.hpp"

namespace rad
{
	// RenderTarget : A Render target that can also be used as a Shader resource view.
	// NOTE : As of now, the RTV that renders into swapchain's backbuffer will not use this implementation : It will remain as is.
	// Also, this will not make use of the Texture class as it is meant for textures that are only SRV's.

	struct RTVertex
	{
		dx::XMFLOAT2 position;
		dx::XMFLOAT2 texCoords;
	};

	// Even though this is *technically* not going to be used by the RenderTarget, it still plays a role in manupilation of how the RenderTarget 
	// is being renderered, hence playing it here.
	struct RTConstantBuffer
	{
		float value;
		dx::XMFLOAT3 padding;
	};

	class RenderTarget
	{
	public:
		void Init(const wrl::ComPtr<ID3D11Device>& device, int width, int height);

		void Bind(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);
		
		void Update(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);

	public:
		VertexBuffer<RTVertex> m_Vertices;
		IndexBuffer m_IndexBuffer;

		D3D11_VIEWPORT m_Viewport{};
		
		ConstantBuffer<RTConstantBuffer> m_RTConstantBuffer;
		
		wrl::ComPtr<ID3D11Texture2D> m_Texture;
		wrl::ComPtr<ID3D11RenderTargetView> m_RTV;
		wrl::ComPtr<ID3D11ShaderResourceView> m_SRV;
	};
}