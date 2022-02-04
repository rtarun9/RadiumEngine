#pragma once

#include "Pch.hpp"

#include "Mesh.hpp"
#include "Graphics/Buffer.hpp"

#include <assimp/scene.h>

namespace rad
{
	struct LoadedTexture
	{
		std::wstring path;
		Texture texture;
	};

	class Model
	{
	public:
		void Init(const wrl::ComPtr<ID3D11Device>& device, const std::wstring& path);
		void Draw(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);
	
		static Model CubeModel(const wrl::ComPtr<ID3D11Device>& device);

	public:
		void ProcessNode(const wrl::ComPtr<ID3D11Device>& device, aiNode* node, const aiScene* scene);

		// Translate aiMesh into custom Mesh class
		void ProcessMesh(const wrl::ComPtr<ID3D11Device>& device, aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> LoadMaterialTextures(const wrl::ComPtr<ID3D11Device>& device, aiMaterial* material, aiTextureType textureType, TextureTypes type);

		void UpdateTransformComponent(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);

	public:
		std::wstring m_ModelDirectory;

		std::vector<LoadedTexture> m_LoadedTextures;

		std::vector<Mesh> m_Meshes;
		Transform m_Transform{};
		PerObjectData m_PerObjectData{};
		ConstantBuffer<PerObjectData> m_PerObjectConstantBuffer{};
	};
}