#include "Graphics/Shader.hpp"

wrl::ComPtr<ID3DBlob> Shader::LoadShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& shaderProfile)
{
	wrl::ComPtr<ID3DBlob> errorBlob;

	UINT flags = 0;

#if _DEBUG
	flags = D3DCOMPILE_DEBUG;
#endif

	HRESULT hr = D3DCompileFromFile(fileName.c_str(), nullptr, nullptr, entryPoint.c_str(), shaderProfile.c_str(), flags, 0, &m_ShaderBlob, &errorBlob);

	if (FAILED(hr))
	{
		if (hr == 0x80070003)
		{
			ErrorMessage(std::string("Could not find file"));
		}
		if (errorBlob.Get())
		{
			std::string errorMessage = (char*)errorBlob->GetBufferPointer();
			ErrorMessage(errorMessage);

			return nullptr;
		}
	}

	return m_ShaderBlob;
}

wrl::ComPtr<ID3DBlob> Shader::GetBytecodeBlob()
{
	return m_ShaderBlob;
}


void VertexShader::Init(wrl::ComPtr<ID3D11Device> device, const std::wstring& fileName, const std::string& entryPoint, const std::string& shaderProfile)
{
	m_ShaderBlob = LoadShader(fileName, entryPoint, shaderProfile);

	ThrowIfFailed(device->CreateVertexShader(m_ShaderBlob->GetBufferPointer(), m_ShaderBlob->GetBufferSize(), nullptr, &m_VertexShader));
}

void VertexShader::Bind(wrl::ComPtr<ID3D11DeviceContext> deviceContext)
{
	deviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
}

void PixelShader::Init(wrl::ComPtr<ID3D11Device> device, const std::wstring& fileName, const std::string& entryPoint, const std::string& shaderProfile)
{
	m_ShaderBlob = LoadShader(fileName, entryPoint, shaderProfile);

	ThrowIfFailed(device->CreatePixelShader(m_ShaderBlob->GetBufferPointer(), m_ShaderBlob->GetBufferSize(), nullptr, &m_PixelShader));
}

void PixelShader::Bind(wrl::ComPtr<ID3D11DeviceContext> deviceContext)
{
	deviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
}