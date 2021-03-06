#pragma once

#include "Pch.hpp"

#include "Graphics/API/Buffer.hpp"
#include "Graphics/API/Texture.hpp"

namespace rad
{
	// NOTE : Will need to make between aiTextureTypes and custom TextureType in future (similar to approach taken for
	//	      WIN32 virtual key codes and custom key types). 
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
		dx::XMFLOAT3 tangent;
		dx::XMFLOAT3 bitangent;
	};

	struct Transform
	{
		dx::XMFLOAT3 translation;
		dx::XMFLOAT3 scale;
		dx::XMFLOAT3 rotation;
	};

	class Mesh
	{
	public:
		void Init(ID3D11Device* device, std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices, std::vector<Texture>&& textures);

		void Draw(ID3D11DeviceContext* deviceContext);

	public:
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		std::vector<Texture> m_Textures;

		VertexBuffer<Vertex> m_VertexBuffer{};
		IndexBuffer m_IndexBuffer{};
		size_t m_IndicesCount{};
	};

}
