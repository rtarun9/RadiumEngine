#include "Graphics/API/Shader.hpp"

namespace rad
{
	wrl::ComPtr<ID3DBlob> Shader::LoadShader(std::wstring_view fileName, std::wstring_view entryPoint, std::wstring_view shaderProfile)
	{
		wrl::ComPtr<ID3DBlob> errorBlob;

		UINT flags = 0;

#if _DEBUG
		flags = D3DCOMPILE_DEBUG;
#endif

		HRESULT hr = D3DCompileFromFile(fileName.data(), nullptr, nullptr, WStringToString(entryPoint.data()).data(), WStringToString(shaderProfile.data()).c_str(), flags, 0, &m_ShaderBlob, &errorBlob);

		if (FAILED(hr))
		{
			if (hr == 0x80070003)
			{
				ErrorMessage(std::wstring(L"Could not find file at path : ") + fileName.data());
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


	void VertexShader::Init(ID3D11Device* device, std::wstring_view  fileName, std::wstring_view  entryPoint, std::wstring_view shaderProfile)
	{
		m_ShaderBlob = LoadShader(fileName, entryPoint, shaderProfile);

		ThrowIfFailed(device->CreateVertexShader(m_ShaderBlob->GetBufferPointer(), m_ShaderBlob->GetBufferSize(), nullptr, &m_VertexShader));
	}

	void VertexShader::Bind(ID3D11DeviceContext* deviceContext)
	{
		deviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
	}

	void PixelShader::Init(ID3D11Device* device, std::wstring_view fileName, std::wstring_view entryPoint, std::wstring_view shaderProfile)
	{
		m_ShaderBlob = LoadShader(fileName, entryPoint, shaderProfile);

		ThrowIfFailed(device->CreatePixelShader(m_ShaderBlob->GetBufferPointer(), m_ShaderBlob->GetBufferSize(), nullptr, &m_PixelShader));
	}

	void PixelShader::Bind(ID3D11DeviceContext* deviceContext)
	{
		deviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
	}

	void ShaderModule::Init(ID3D11Device* device, InputLayoutType inputLayoutType, std::wstring_view vsFilePath, std::wstring_view psFilePath)
	{
		if (inputLayoutType == InputLayoutType::DefaultInput)
		{
			inputLayout.AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT);
			inputLayout.AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT);
			inputLayout.AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
			inputLayout.AddInputElement("TANGENT", DXGI_FORMAT_R32G32B32_FLOAT);
			inputLayout.AddInputElement("BITANGENT", DXGI_FORMAT_R32G32B32_FLOAT);
		}
		else if (inputLayoutType == InputLayoutType::RenderTargetInput)
		{
			inputLayout.AddInputElement("POSITION", DXGI_FORMAT_R32G32_FLOAT);
			inputLayout.AddInputElement("TEXCOORD", DXGI_FORMAT_R32G32_FLOAT);
		}
		
		vertexShader.Init(device, vsFilePath);
		pixelShader.Init(device, psFilePath);
		inputLayout.Init(device, vertexShader.GetBytecodeBlob().Get());
	}

	void ShaderModule::Bind(ID3D11DeviceContext* deviceContext)
	{
		inputLayout.Bind(deviceContext);
		vertexShader.Bind(deviceContext);
		pixelShader.Bind(deviceContext);
	}
}
