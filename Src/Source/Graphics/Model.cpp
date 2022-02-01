#include "Graphics/Model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace rad
{
	void Model::Init(const wrl::ComPtr<ID3D11Device>& device, const std::wstring& path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(WStringToString(path), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			ErrorMessage(L"Cannot load model with path : " + path);
		}

		m_Transform.rotation = dx::XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Transform.scale = dx::XMFLOAT3(1.0f, 1.0f, 1.0f);
		m_Transform.translation = dx::XMFLOAT3(0.0f, 0.0f, 0.0f);

		m_PerObjectConstantBuffer.Init(device);

		m_ModelDirectory = path.substr(0, path.find_last_of(L'/') + 1);

		ProcessNode(device, scene->mRootNode, scene);
	}

	void Model::Draw(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
	{
		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			m_Meshes[i].Draw(deviceContext);
		}
	}

	void Model::ProcessNode(const wrl::ComPtr<ID3D11Device>& device, aiNode* node, const aiScene* scene)
	{
		// Process all meshes of node.
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(device, mesh, scene);
		}

		// Process all children of current node.
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(device, node->mChildren[i], scene);
		}
	}

	void Model::ProcessMesh(const wrl::ComPtr<ID3D11Device>& device, aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex = {};

			vertex.position.x = mesh->mVertices[i].x;
			vertex.position.y = mesh->mVertices[i].y;
			vertex.position.z = mesh->mVertices[i].z;

			vertex.normal.x = mesh->mNormals[i].x;
			vertex.normal.y = mesh->mNormals[i].y;
			vertex.normal.z = mesh->mNormals[i].z;

			// NOTE : ASSIMP CAN HAVE 8 TEX COORDS PER VERTEX, BUT USING ONLY FIRST FOR NOW
			if (mesh->mTextureCoords[0])
			{
				vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
				vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
			}
			else
			{
				vertex.texCoords.x = 0;
				vertex.texCoords.y = 0;
			}

			vertices.push_back(vertex);
		}

		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		std::vector<Texture> textures;

		// As of now, there will only be 4 textures (one for each type of map)
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<Texture> diffuseMaps = LoadMaterialTextures(device, material, aiTextureType_DIFFUSE, TextureTypes::TextureDiffuse);
			if (!diffuseMaps.size())
			{
				textures.push_back(Texture::DefaultTexture(device));
			}
			else
			{
				textures.push_back(diffuseMaps[0]);
			}

			std::vector<Texture>  specularMaps = LoadMaterialTextures(device, material, aiTextureType_SPECULAR, TextureTypes::TextureSpecular);
			if (!specularMaps.size())
			{
				textures.push_back(Texture::DefaultTexture(device));
			}
			else
			{
				textures.push_back(specularMaps[0]);
			}

			std::vector<Texture> normalMaps = LoadMaterialTextures(device, material, aiTextureType_NORMALS, TextureTypes::TextureNormal);
			if (!normalMaps.size())
			{
				textures.push_back(Texture::DefaultTexture(device));
			}
			else
			{
				textures.push_back(normalMaps[0]);
			}

			std::vector<Texture> heightMaps = LoadMaterialTextures(device, material, aiTextureType_HEIGHT, TextureTypes::TextureHeight);
			if (!heightMaps.size())
			{
				textures.push_back(Texture::DefaultTexture(device));
			}
			else
			{
				textures.push_back(heightMaps[0]);
			}
		}
		else
		{
			// Can be optimized further.
			textures.emplace_back(Texture::DefaultTexture(device));
			textures.emplace_back(Texture::DefaultTexture(device));
			textures.emplace_back(Texture::DefaultTexture(device));
			textures.emplace_back(Texture::DefaultTexture(device));
		}

		Mesh newMesh{};
		newMesh.Init(device, vertices, indices, textures);
		m_Meshes.push_back(newMesh);
	}

	std::vector<Texture> Model::LoadMaterialTextures(const wrl::ComPtr<ID3D11Device>& device, aiMaterial* material, aiTextureType textureType, TextureTypes type)
	{
		std::vector<Texture> textures;

		for (uint32_t i = 0; i < material->GetTextureCount(textureType); i++)
		{
			aiString str;
			material->GetTexture(textureType, i, &str);

			bool alreadyAdded = false;
			for (uint32_t j = 0; j < m_LoadedTextures.size(); j++)
			{
				if (!std::strcmp(WStringToString(m_LoadedTextures[j].path).data(), str.C_Str()))
				{
					textures.push_back(m_LoadedTextures[j].texture);
					alreadyAdded = true;
					break;
				}
			}
			
			if (!alreadyAdded)
			{
				Texture texture;

				std::wstring path = m_ModelDirectory + StringToWString(str.C_Str());

				texture.Init(device, path);
				textures.push_back(texture);

				m_LoadedTextures.push_back({ StringToWString(str.C_Str()), texture });
			}
		}

		return textures;
	}

	void Model::UpdateTransformComponent(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
	{
		// NOTE : Rotation does not work as expected when rotation is done after translation.
		dx::XMVECTOR rotationVector = dx::XMVectorSet(m_Transform.rotation.x, m_Transform.rotation.y, m_Transform.rotation.z, 0.0f);
		dx::XMMATRIX transform = dx::XMMatrixTranslation(m_Transform.translation.x, m_Transform.translation.y, m_Transform.translation.z) *
			dx::XMMatrixRotationRollPitchYawFromVector(rotationVector) *
			dx::XMMatrixScaling(m_Transform.scale.x, m_Transform.scale.y, m_Transform.scale.z);

		m_PerObjectData.modelMatrix = transform;
		m_PerObjectData.inverseTransposedModelMatrix = dx::XMMatrixInverse(nullptr, dx::XMMatrixTranspose(transform));

		m_PerObjectConstantBuffer.Update(deviceContext, m_PerObjectData);
	}

	Model Model::CubeModel(const wrl::ComPtr<ID3D11Device>& device)
	{
		Model cube;
		cube.Init(device, L"../Assets/Models/Cube/cube.obj");

		return cube;
	}
}
