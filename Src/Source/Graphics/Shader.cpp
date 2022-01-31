#include "Graphics/Shader.hpp"

namespace rad
{
	wrl::ComPtr<ID3DBlob> Shader::LoadShader(const std::wstring& fileName, const std::wstring& entryPoint, const std::wstring& shaderProfile)
	{
		wrl::ComPtr<ID3DBlob> errorBlob;

		UINT flags = 0;

#if _DEBUG
		flags = D3DCOMPILE_DEBUG;
#endif

		auto x = WStringToString(entryPoint);
		HRESULT hr = D3DCompileFromFile(fileName.c_str(), nullptr, nullptr, WStringToString(entryPoint).c_str(), WStringToString(shaderProfile).c_str(), flags, 0, &m_ShaderBlob, &errorBlob);

		if (FAILED(hr))
		{
			if (hr == 0x80070003)
			{
				ErrorMessage(std::wstring(L"Could not find file at path : ") + fileName);
			}
			if (errorBlob.Get())
			{
				std::string errorMessage = (char*)errorBlob->GetBufferPointer();
				ErrorMessage(StringToWString(errorMessage));

				return nullptr;
			}
		}

		return m_ShaderBlob;
	}

	wrl::ComPtr<ID3DBlob> Shader::GetBytecodeBlob()
	{
		return m_ShaderBlob;
	}


	void VertexShader::Init(const wrl::ComPtr<ID3D11Device>& device, const std::wstring& fileName, const std::wstring& entryPoint, const std::wstring& shaderProfile)
	{
		m_ShaderBlob = LoadShader(fileName, entryPoint, shaderProfile);

		ThrowIfFailed(device->CreateVertexShader(m_ShaderBlob->GetBufferPointer(), m_ShaderBlob->GetBufferSize(), nullptr, &m_VertexShader));
	}

	void VertexShader::Bind(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
	{
		deviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
	}

	void PixelShader::Init(const wrl::ComPtr<ID3D11Device>& device, const std::wstring& fileName, const std::wstring& entryPoint, const std::wstring& shaderProfile)
	{
		m_ShaderBlob = LoadShader(fileName, entryPoint, shaderProfile);

		ThrowIfFailed(device->CreatePixelShader(m_ShaderBlob->GetBufferPointer(), m_ShaderBlob->GetBufferSize(), nullptr, &m_PixelShader));
	}

	void PixelShader::Bind(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
	{
		deviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
	}

	void ShaderModule::Bind(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
	{
		vertexShader.Bind(deviceContext);
		pixelShader.Bind(deviceContext);
	}
}
