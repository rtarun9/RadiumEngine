#pragma once

#include "Pch.hpp"

#include "Buffer.hpp"

// Will be constant buffer
struct LightData
{
	// lightPosition is float4 due to padding reasons.
	float ambientStrength;
	dx::XMFLOAT3 lightColor;

	dx::XMFLOAT4 lightDirection;
};

class DirectionalLight
{
public:
	void Init(const wrl::ComPtr<ID3D11Device>& device);
	void Update(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);

public:
	LightData m_LightData{};
	ConstantBuffer<LightData> m_LightConstantBuffer;
};