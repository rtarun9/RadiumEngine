#include "Graphics/Lights.hpp"

namespace rad
{
	void DirectionalLight::Init(const wrl::ComPtr<ID3D11Device>& device)
	{
		m_LightData.ambientStrength = 0.03f;
		m_LightData.lightColor = dx::XMFLOAT3(1.0f, 1.0f, 1.0f);
		m_LightData.lightDirection = dx::XMFLOAT4(0.0f, -10.0f, 10.0f, 0.0f);

		m_LightConstantBuffer.Init(device);

		m_PerFrameConstantBuffer.Init(device);
	}


	void DirectionalLight::Update(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
	{
		m_LightConstantBuffer.Update(deviceContext, m_LightData);
	}

	void DirectionalLight::UpdatePerFrameData(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
	{
		dx::XMVECTOR eyePosition = dx::XMLoadFloat4(&m_LightData.lightDirection);
		m_PerFrameConstantBuffer.m_Data.lightViewMatrix = dx::XMMatrixLookAtLH(eyePosition, dx::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		m_PerFrameConstantBuffer.m_Data.lightProjectionMatrix = dx::XMMatrixOrthographicOffCenterLH(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
	
		m_PerFrameConstantBuffer.Update(deviceContext);
	}

}

