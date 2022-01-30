#include "Graphics/Lights.hpp"

namespace rad
{
	void DirectionalLight::Init(const wrl::ComPtr<ID3D11Device>& device)
	{
		m_LightConstantBuffer.Init(device);
	}

	void DirectionalLight::Update(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
	{
		m_LightConstantBuffer.Update(deviceContext, m_LightData);
	}

}

