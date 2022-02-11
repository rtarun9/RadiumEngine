#pragma once

#include "Pch.hpp"

#include "InputLayout.hpp"

namespace rad
{
	class Shader
	{
	public:
		[[nodiscard]]
		wrl::ComPtr<ID3DBlob> LoadShader(std::wstring_view fileName, std::wstring_view entryPoint, std::wstring_view shaderProfile);

		[[nodiscard]]
		wrl::ComPtr<ID3DBlob> GetBytecodeBlob();

	protected:
		wrl::ComPtr<ID3DBlob> m_ShaderBlob;
	};

	class VertexShader : public Shader
	{
	public:
		void Init(ID3D11Device* device, std::wstring_view  fileName, std::wstring_view entryPoint = L"VsMain", std::wstring_view shaderProfile = L"vs_5_0");

		void Bind(ID3D11DeviceContext* deviceContext);

	public:
		wrl::ComPtr<ID3D11VertexShader> m_VertexShader;
	};

	class PixelShader : public Shader
	{
	public:
		void Init(ID3D11Device* device, std::wstring_view fileName, std::wstring_view entryPoint = L"PsMain", std::wstring_view = L"ps_5_0");

		void Bind(ID3D11DeviceContext* deviceContext);

	public:
		wrl::ComPtr<ID3D11PixelShader> m_PixelShader;
	};

	struct ShaderModule
	{
		VertexShader vertexShader{};
		PixelShader pixelShader{};

		InputLayout inputLayout{};

		void Init(ID3D11Device* device, InputLayoutType inputLayoutType, std::wstring_view vsFilePath, std::wstring_view psFilePath);
		void Bind(ID3D11DeviceContext* deviceContext);
	};
}

