#include "Model/Model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <imgui.h>

namespace rad
{
	void Model::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& path)
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

		m_PerObjectConstantBuffer.m_Data.color = dx::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_PerObjectConstantBuffer.Init(device);

		m_ModelDirectory = path.substr(0, path.find_last_of(L'/') + 1);

		ProcessNode(device, deviceContext, scene->mRootNode, scene);

		RAD_CORE_INFO("Loaded model with path : {0}", WStringToString(path));
	}

	void Model::Draw(ID3D11DeviceContext* deviceContext)
	{
		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			m_Meshes[i].Draw(deviceContext);
		}
	}

	void Model::ProcessNode(ID3D11Device* device, ID3D11DeviceContext* deviceContext, aiNode* node, const aiScene* scene)
	{
		// Process all meshes of node.
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(device, deviceContext, mesh, scene);
		}

		// Process all children of current node.
		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(device, deviceContext, node->mChildren[i], scene);
		}
	}

	void Model::ProcessMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		vertices.reserve(mesh->mNumVertices);

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
			
				vertex.tangent.x = mesh->mTangents[i].x;
				vertex.tangent.y = mesh->mTangents[i].y;
				vertex.tangent.z = mesh->mTangents[i].z;

				vertex.bitangent.x = mesh->mBitangents[i].x;
				vertex.bitangent.y = mesh->mBitangents[i].y;
				vertex.bitangent.z = mesh->mBitangents[i].z;
			}
			else
			{
				vertex.texCoords.x = 0;
				vertex.texCoords.y = 0;

				vertex.tangent.x = 0;
				vertex.tangent.y = 0;
				vertex.tangent.z = 0;

				vertex.bitangent.x = 0;
				vertex.bitangent.y = 0;
				vertex.bitangent.z = 0;
			}

			vertices.push_back(std::move(vertex));
		}

		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(std::move(face.mIndices[j]));
			}
		}

		std::vector<Texture> textures;
		textures.reserve(TextureCount);

		// As of now, there will only be 4 textures (one for each type of map)
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<Texture> diffuseMaps = LoadMaterialTextures(device, deviceContext, material, aiTextureType_DIFFUSE, TextureTypes::TextureDiffuse);
			if (!diffuseMaps.size())
			{
				textures.push_back(Texture::DefaultTexture(device, deviceContext));
				RAD_CORE_WARN("Model with path : {0} does not have Diffuse Maps", WStringToString(m_ModelDirectory));
			}
			else
			{
				textures.push_back(std::move(diffuseMaps[0]));
			}

			std::vector<Texture>  specularMaps = LoadMaterialTextures(device, deviceContext ,material, aiTextureType_SPECULAR, TextureTypes::TextureSpecular);
			if (!specularMaps.size())
			{
				textures.push_back(Texture::DefaultTexture(device, deviceContext));
				//RAD_CORE_WARN("Model with path : {0} does not have Specular Maps", WStringToString(m_ModelDirectory));
			}
			else
			{
				textures.push_back(std::move(specularMaps[0]));
			}

			std::vector<Texture> normalMaps = LoadMaterialTextures(device, deviceContext, material, aiTextureType_NORMALS, TextureTypes::TextureNormal);
			if (!normalMaps.size())
			{
				textures.push_back(Texture::DefaultTexture(device, deviceContext));
				//RAD_CORE_WARN("Model with path : {0} does not have Normal Maps", WStringToString(m_ModelDirectory));
			}
			else
			{
				textures.push_back(std::move(normalMaps[0]));
			}

			std::vector<Texture> heightMaps = LoadMaterialTextures(device, deviceContext, material, aiTextureType_HEIGHT, TextureTypes::TextureHeight);
			if (!heightMaps.size())
			{
				textures.push_back(Texture::DefaultTexture(device, deviceContext));
				//RAD_CORE_WARN("Model with path : {0} does not have Height Maps", WStringToString(m_ModelDirectory));
			}
			else
			{
				textures.push_back(std::move(heightMaps[0]));
			}
		}
		else
		{
			// Can be optimized further.
			textures.emplace_back(Texture::DefaultTexture(device, deviceContext));
			textures.emplace_back(Texture::DefaultTexture(device, deviceContext));
			textures.emplace_back(Texture::DefaultTexture(device, deviceContext));
			textures.emplace_back(Texture::DefaultTexture(device, deviceContext));
		}

		Mesh newMesh{};
		newMesh.Init(device, std::move(vertices), std::move(indices), std::move(textures));
		m_Meshes.push_back(std::move(newMesh));
	}

	std::vector<Texture> Model::LoadMaterialTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContext, aiMaterial* material, aiTextureType textureType, TextureTypes type)
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
					textures.push_back(std::move(m_LoadedTextures[j].texture));
					alreadyAdded = true;
					break;
				}
			}
			
			if (!alreadyAdded)
			{
				Texture texture;

				std::wstring path = m_ModelDirectory + StringToWString(str.C_Str());

				if (type == TextureTypes::TextureDiffuse || type == TextureTypes::TextureSpecular)
				{
					texture.Init(device, deviceContext, path, true);
				}
				else
				{
					texture.Init(device, deviceContext, path, false);
				}

				textures.push_back(texture);

				m_LoadedTextures.push_back({ StringToWString(str.C_Str()), texture });
			}
		}

		return textures;
	}

	void Model::UpdateTransformComponent(ID3D11DeviceContext* deviceContext)
	{
		// NOTE : Rotation does not work as expected when rotation is done after translation.
		dx::XMVECTOR rotationVector = dx::XMVectorSet(m_Transform.rotation.x, m_Transform.rotation.y, m_Transform.rotation.z, 0.0f);
		dx::XMMATRIX transform = dx::XMMatrixTranslation(m_Transform.translation.x, m_Transform.translation.y, m_Transform.translation.z) *
			dx::XMMatrixRotationRollPitchYawFromVector(rotationVector) *
			dx::XMMatrixScaling(m_Transform.scale.x, m_Transform.scale.y, m_Transform.scale.z);

		m_PerObjectConstantBuffer.m_Data.modelMatrix = transform;
		m_PerObjectConstantBuffer.m_Data.inverseTransposedModelMatrix = dx::XMMatrixInverse(nullptr, dx::XMMatrixTranspose(transform));

		m_PerObjectConstantBuffer.Update(deviceContext);
	}

	void Model::UpdateData()
	{
		// NOTE : scaling on Y not working as expected.
		ImGui::SliderFloat3("Translate", &m_Transform.translation.x, -10.0f, 10.0f);
		ImGui::SliderFloat3("Rotate", &m_Transform.rotation.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("Scale", &m_Transform.scale.x, 0.1f, 50.0f);
		ImGui::SliderFloat3("Color", &m_PerObjectConstantBuffer.m_Data.color.x, 0.0f, 5.0f);
		ImGui::TreePop();
	}

	Model Model::CubeModel(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	{
		Model cube;
		cube.Init(device, deviceContext, L"../Assets/Models/Cube/glTF/Cube.gltf");

		return cube;
	}
}
