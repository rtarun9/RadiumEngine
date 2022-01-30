#pragma once

#include <assimp/scene.h>

#include "Pch.hpp"

#include "Graphics/Buffer.hpp"

#include "Mesh.hpp"

namespace rad
{
	//// Uncomment when Mesh class is removed (the one with tinyobjloader)
	//struct Vertex
	//{
	//	dx::XMFLOAT3 position;
	//	dx::XMFLOAT3 normal;
	//	dx::XMFLOAT2 texCoords;
	//};

	// Will replace other Mesh class soon, hence the underscore.
	class _Mesh
	{
	public:
		void Init(const wrl::ComPtr<ID3D11Device>& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

		void Draw(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);

	private:
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;

		VertexBuffer<Vertex> m_VertexBuffer;
		IndexBuffer m_IndexBuffer;
	};

	class Model
	{
	public:
		void Init(const wrl::ComPtr<ID3D11Device>& device, const std::string& path);
		void Draw(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);
	
	private:
		void ProcessNode(const wrl::ComPtr<ID3D11Device>& device, aiNode* node, const aiScene* scene);

		// Translate aiMesh into custom Mesh class
		void ProcessMesh(const wrl::ComPtr<ID3D11Device>& device, aiMesh* mesh, const aiScene* scene);

	private:
		std::vector<_Mesh> m_Meshes;
	};
}