#pragma once

#include "Pch.hpp"

#include "Graphics/Buffer.hpp"

namespace rad
{
	struct Vertex
	{
		dx::XMFLOAT3 position;
		dx::XMFLOAT3 normal;
		dx::XMFLOAT2 texCoords;
	};

	struct Transform
	{
		dx::XMFLOAT3 translation;
		dx::XMFLOAT3 scale;
		dx::XMFLOAT3 rotation;
	};

	struct PerObjectData
	{
		dx::XMMATRIX modelMatrix;
		dx::XMMATRIX inverseTransposedModelMatrix;

		// Temporarily used, to be removed soon.
		dx::XMFLOAT3 color;
	};

	class Mesh
	{
	public:
		void Init(const wrl::ComPtr<ID3D11Device>& device, const std::string& filePath);
		void Draw(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);
		void UpdateTransformComponent(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);

	public:
		VertexBuffer<Vertex> m_VertexBuffer;

		Transform m_Transform{};
		PerObjectData m_PerObjectData{};
		ConstantBuffer<PerObjectData> m_TransformConstantBuffer{};
	};

}
