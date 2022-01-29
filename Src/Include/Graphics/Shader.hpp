#pragma once

#include "Pch.hpp"

class Shader
{
public:
	[[nodiscard]]
	wrl::ComPtr<ID3DBlob> LoadShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& shaderProfile);

	[[nodiscard]]
	wrl::ComPtr<ID3DBlob> GetBytecodeBlob();

public:
	wrl::ComPtr<ID3DBlob> m_ShaderBlob;
};

class VertexShader : public Shader
{
public:
	void Init(const wrl::ComPtr<ID3D11Device>& device, const std::wstring& fileName, const std::string& entryPoint, const std::string& shaderProfile = "vs_5_0");

	void Bind(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);

public:
	wrl::ComPtr<ID3D11VertexShader> m_VertexShader;
};

class PixelShader : public Shader
{
public:
	void Init(const wrl::ComPtr<ID3D11Device>& device, const std::wstring& fileName, const std::string& entryPoint, const std::string& shaderProfile = "ps_5_0");

	void Bind(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);

public:
	wrl::ComPtr<ID3D11PixelShader> m_PixelShader;
};

struct ShaderModule
{
	VertexShader vertexShader;
	PixelShader pixelShader;
};