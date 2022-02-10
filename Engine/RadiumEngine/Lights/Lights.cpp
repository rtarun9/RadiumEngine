#include "Lights/Lights.hpp"

#include <imgui.h>

namespace rad
{
	void DirectionalLight::Init(ID3D11Device* device)
	{
		m_LightConstantBuffer.m_Data.ambientStrength = 0.04f;
		m_LightConstantBuffer.m_Data.lightColor = dx::XMFLOAT3(1.0f, 1.0f, 1.0f);
		m_LightConstantBuffer.m_Data.lightDirection = dx::XMFLOAT3(0.0f, sin(dx::XMConvertToRadians(m_SunAngle)), cos(dx::XMConvertToRadians(m_SunAngle)));
		m_LightConstantBuffer.m_Data.lightStrength = 1.0f;
	
		m_LightConstantBuffer.Init(device);

		m_PerFrameConstantBuffer.Init(device);
	}


	void DirectionalLight::Update(ID3D11DeviceContext* deviceContext)
	{
		m_LightConstantBuffer.Update(deviceContext);

		m_LightConstantBuffer.m_Data.lightDirection = dx::XMFLOAT3(0.0f, sin(dx::XMConvertToRadians(m_SunAngle)), cos(dx::XMConvertToRadians(m_SunAngle)));
		dx::XMFLOAT4 lightDirection = dx::XMFLOAT4(m_LightConstantBuffer.m_Data.lightDirection.x, m_LightConstantBuffer.m_Data.lightDirection.y, m_LightConstantBuffer.m_Data.lightDirection.z, 0.0f);

		dx::XMVECTOR eyePosition = dx::XMLoadFloat4(&lightDirection);
		eyePosition = dx::XMVector4Normalize(eyePosition);

		dx::XMVECTOR lightPosition = m_LightTarget - eyePosition * m_BackOffDistance;
		
		m_PerFrameConstantBuffer.m_Data.lightViewMatrix = dx::XMMatrixLookAtLH(lightPosition,m_LightTarget, dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		m_PerFrameConstantBuffer.m_Data.lightProjectionMatrix = dx::XMMatrixOrthographicOffCenterLH(-m_Extents, m_Extents, -m_Extents, m_Extents, m_NearPlane, m_FarPlane);

		m_PerFrameConstantBuffer.Update(deviceContext);
	}

	void DirectionalLight::UpdateData()
	{
		ImGui::Begin("Light Properties");
		if (ImGui::TreeNode("Directional Light"))
		{
			ImGui::SliderFloat("Ambient Strength", &m_LightConstantBuffer.m_Data.ambientStrength, 0.0f, 10.0f);
			ImGui::SliderFloat("Light Strength", &m_LightConstantBuffer.m_Data.lightStrength, 0.5f, 50.0f);
			ImGui::ColorEdit3("Color", &m_LightConstantBuffer.m_Data.lightColor.x);
			ImGui::SliderFloat("Sun Direction", &m_SunAngle, -90.0f, 90.0f);
			ImGui::SliderFloat("Extents", &m_Extents, 30.0f, 200.0f);
			ImGui::SliderFloat("Near Plane", &m_NearPlane, 0.1f, 1000.0f);
			ImGui::SliderFloat("Far Plane", &m_FarPlane, 10.0f, 1000.0f);
			ImGui::SliderFloat("Back off distance", &m_BackOffDistance, 0.6f, 500.0f);
			ImGui::TreePop();
		}

		ImGui::End();
	}

	void PointLight::Init(ID3D11Device* device)
	{
		m_LightConstantBuffer.m_Data.ambientStrength = 0.04f;
		m_LightConstantBuffer.m_Data.lightColor = dx::XMFLOAT3(1.0f, 1.0f, 1.0f);
		m_LightConstantBuffer.m_Data.lightDirection = dx::XMFLOAT3(0.0f, 10.0f, 0.0f);
		m_LightConstantBuffer.m_Data.lightStrength = 1.0f;

		m_LightConstantBuffer.Init(device);
	}


	void PointLight::Update(ID3D11DeviceContext* deviceContext)
	{
		m_LightConstantBuffer.Update(deviceContext);
	}

	void PointLight::UpdateData()
	{
		ImGui::Begin("Light Properties");
		if (ImGui::TreeNode("Point Light"))
		{
			ImGui::SliderFloat("Ambient Strength", &m_LightConstantBuffer.m_Data.ambientStrength, 0.0f, 10.0f);
			ImGui::SliderFloat("Light Strength", &m_LightConstantBuffer.m_Data.lightStrength, 0.5f, 50.0f);
			ImGui::ColorEdit3("Color", &m_LightConstantBuffer.m_Data.lightColor.x);
			ImGui::SliderFloat3("Light position", &m_LightConstantBuffer.m_Data.lightDirection.x, -10.0f, 50.0f);
			ImGui::TreePop();
		}

		ImGui::End();
	}
}

