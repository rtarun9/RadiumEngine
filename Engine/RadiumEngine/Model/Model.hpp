#pragma once

#include "Pch.hpp"

#include "Mesh.hpp"
#include "Graphics/API/Buffer.hpp"

#include <assimp/scene.h>

namespace rad
{
	struct LoadedTexture
	{
		std::wstring path;
		Texture texture;
	};


	struct PerObjectData
	{
		dx::XMMATRIX modelMatrix;
		dx::XMMATRIX inverseTransposedModelMatrix;
		dx::XMFLOAT4 color;
	};

	class Model
	{
	public:
		void Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& path);
		void Draw(ID3D11DeviceContext* deviceContext);
	
		static Model CubeModel(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	public:
		void ProcessNode(ID3D11Device* device, ID3D11DeviceContext* deviceContext, aiNode* node, const aiScene* scene);

		// Translate aiMesh into custom Mesh class
		void ProcessMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> LoadMaterialTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContext, aiMaterial* material, aiTextureType textureType, TextureTypes type);

		void UpdateTransformComponent(ID3D11DeviceContext* deviceContext);

		void UpdateData();

	public:
		std::wstring m_ModelDirectory;

		std::vector<LoadedTexture> m_LoadedTextures;

		std::vector<Mesh> m_Meshes;

		Transform m_Transform{};
		ConstantBuffer<PerObjectData> m_PerObjectConstantBuffer{};
	};
}