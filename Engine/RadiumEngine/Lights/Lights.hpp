#pragma once

#include "Pch.hpp"

#include "Graphics/API/Buffer.hpp"

namespace rad
{
	// Will be constant buffer
	struct LightData
	{
		float ambientStrength;
		dx::XMFLOAT3 lightColor;

		float lightStrength;
		dx::XMFLOAT3 lightDirection;
	};

	// Used by and for the shadow pass.
	struct LightPerFrameData
	{
		dx::XMMATRIX lightViewMatrix;
		dx::XMMATRIX lightProjectionMatrix;
	};

	// The current parameters are hand chosen for the specific scene. This might change once I add CSM, but unsure.
	class DirectionalLight
	{
	public:
		void Init(ID3D11Device *device);
		void Update(ID3D11DeviceContext* deviceContext);

		void UpdateData();

	public:
		static constexpr float SHADOW_MAP_DIMENSION = 2048;
		ConstantBuffer<LightData> m_LightConstantBuffer;
		ConstantBuffer<LightPerFrameData> m_PerFrameConstantBuffer;

		float m_BackOffDistance{ 200.0f };
		float m_SunAngle{ -90.0f };
		float m_Extents{ 99.0f };
		float m_NearPlane{ 1.0f };
		float m_FarPlane{ 480.0f };

		dx::XMVECTOR m_LightTarget{ dx::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f) };
	};

	// The current parameters are hand chosen for the specific scene. This might change once I add CSM, but unsure.
	class PointLight
	{
	public:
		void Init(ID3D11Device* device);
		void Update(ID3D11DeviceContext* deviceContext);

		void UpdateData();

	public:
		ConstantBuffer<LightData> m_LightConstantBuffer;
	};
}
