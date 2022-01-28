#include "Graphics/Lights.hpp"

void DirectionalLight::Update(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
{
	m_LightConstantBuffer.Update(deviceContext, m_LightData);
}
