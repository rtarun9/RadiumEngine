#include "Mesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION 
#include "tiny_obj_loader.h"

void Mesh::Init(const wrl::ComPtr<ID3D11Device>& device, const char* filePath)
{
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig readerConfig;

    if (!reader.ParseFromFile(filePath, readerConfig)) 
    {
        ErrorMessage("Could not find mesh at path : " + std::string(filePath));
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

            // per-face material
//            shapes[s].mesh.material_ids[f];
        }
    }

    m_VertexBuffer.Init(device, vertices);
}

void Mesh::Draw(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext)
{
    deviceContext->Draw(m_VertexBuffer.m_VerticesCount, 0);
}
