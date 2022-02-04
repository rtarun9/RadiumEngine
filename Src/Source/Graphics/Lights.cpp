#include "Graphics/Lights.hpp"

namespace rad
{
	void DirectionalLight::Init(const wrl::ComPtr<ID3D11Device>& device)
	{
		m_LightData.ambientStrength = 0.04f;
		m_LightData.lightColor = dx::XMFLOAT3(1.0f, 1.0f, 1.0f);
		m_LightData.lightDirection = dx::XMFLOAT3(0.0f, sin(dx::XMConvertToRadians(m_SunAngle)), cos(dx::XMConvertToRadians(m_SunAngle)));
		m_LightData.lightStrength = 1.0f;
		m_LightConstantBuffer.Init(device);

		m_PerFrameConstantBuffer.Init(device);
	}


	void DirectionalLight::Update(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
	{
		m_LightConstantBuffer.Update(deviceContext, m_LightData);

		m_LightData.lightDirection = dx::XMFLOAT3(0.0f, sin(dx::XMConvertToRadians(m_SunAngle)), cos(dx::XMConvertToRadians(m_SunAngle)));
		dx::XMFLOAT4 lightDirection = dx::XMFLOAT4(m_LightData.lightDirection.x, m_LightData.lightDirection.y, m_LightData.lightDirection.z, 0.0f);

		dx::XMVECTOR eyePosition = dx::XMLoadFloat4(&lightDirection);
		eyePosition = dx::XMVector4Normalize(eyePosition);

		dx::XMVECTOR lightPosition = m_LightTarget - eyePosition * m_BackOffDistance;
		

		m_PerFrameConstantBuffer.m_Data.lightViewMatrix = dx::XMMatrixLookAtLH(lightPosition,m_LightTarget, dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		m_PerFrameConstantBuffer.m_Data.lightProjectionMatrix = dx::XMMatrixOrthographicOffCenterLH(-m_Extents, m_Extents, -m_Extents, m_Extents, m_NearPlane, m_FarPlane);

		m_PerFrameConstantBuffer.Update(deviceContext);
	}
}

