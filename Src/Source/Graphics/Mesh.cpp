#include "Graphics/Mesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION 
#include "tiny_obj_loader.h"

void Mesh::Init(const wrl::ComPtr<ID3D11Device>& device, const std::string& filePath)
{
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig readerConfig;

    if (!reader.ParseFromFile(filePath, readerConfig))
    {
        ErrorMessage(std::wstring(L"Could not find mesh at path: ") + StringToWString(filePath));
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    std::vector<Vertex> vertices;

    for (size_t s = 0; s < shapes.size(); s++)
    {
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            for (size_t v = 0; v < fv; v++)
            {
                Vertex vertex = {};
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

                tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

                vertex.position = { vx, vy, vz };
                vertex.normal = { nx, ny, nz };
                vertex.texCoords = { tx, ty };

                vertices.push_back(vertex);
            }
            index_offset += fv;
        }
    }

    m_Transform.rotation = dx::XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_Transform.scale = dx::XMFLOAT3(1.0f, 1.0f, 1.0f);
    m_Transform.translation = dx::XMFLOAT3(0.0f, 0.0f, 0.0f);

    m_PerObjectData.color = dx::XMFLOAT3(1.0f, 1.0f, 1.0f);

    m_TransformConstantBuffer.Init(device);
    m_VertexBuffer.Init(device, vertices);
}

void Mesh::UpdateTransformComponent(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
{
    // NOTE : Rotation does not work as expected when rotation is done after translation.
    dx::XMVECTOR rotationVector = dx::XMVectorSet(m_Transform.rotation.x, m_Transform.rotation.y, m_Transform.rotation.z, 0.0f);
    dx::XMMATRIX transform = dx::XMMatrixTranslation(m_Transform.translation.x, m_Transform.translation.y, m_Transform.translation.z) *
        dx::XMMatrixRotationRollPitchYawFromVector(rotationVector) * 
        dx::XMMatrixScaling(m_Transform.scale.x, m_Transform.scale.y, m_Transform.scale.z);

    m_PerObjectData.modelMatrix = transform;
    m_PerObjectData.inverseTransposedModelMatrix = dx::XMMatrixInverse(nullptr, dx::XMMatrixTranspose(transform));

    m_TransformConstantBuffer.Update(deviceContext, m_PerObjectData);
}

void Mesh::Draw(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
{
    deviceContext->Draw(m_VertexBuffer.m_VerticesCount, 0);
}