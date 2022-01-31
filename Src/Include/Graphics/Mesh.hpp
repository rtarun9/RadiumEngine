#pragma once

#include "Pch.hpp"

#include "Graphics/Buffer.hpp"
#include "Graphics/Texture.hpp"

namespace rad
{
	// NOTE : Will need to make between aiTextureTypes and custom TextureType in future (similar to approach taken for
	//	      WIN32 virtual key codes and custom key types. 
	enum TextureTypes
	{
		TextureDiffuse,
		TextureSpecular,
		TextureNormal,
		TextureHeight,
		TextureCount
	};

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
	};

	class Mesh
	{
	public:
		void Init(const wrl::ComPtr<ID3D11Device>& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<Texture>& textures);

		void Draw(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);

	private:
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		std::vector<Texture> m_Textures;

		VertexBuffer<Vertex> m_VertexBuffer{};
		IndexBuffer m_IndexBuffer{};
	};

}
