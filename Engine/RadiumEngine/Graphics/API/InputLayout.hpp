#pragma once

#include "Pch.hpp"

namespace rad
{
	enum class InputLayoutType
	{
		DefaultInput,
		RenderTargetInput
	};

	class InputLayout
	{
	public:
		// NOTE : Changing semanticName to type std::wstring is causing some issues due to which it is still using const char *unlike other wrapper classes.
		// Once solution is found, it too will use std::wstring again.
		void AddInputElement(const char *semanticName, DXGI_FORMAT format);

		void Init(ID3D11Device* device, ID3DBlob* vertexBlob);

		void Bind(ID3D11DeviceContext* deviceContext);

	public:
		wrl::ComPtr<ID3D11InputLayout> m_InputLayout;

		std::vector<D3D11_INPUT_ELEMENT_DESC> m_InputElementDescs;
	};
}
