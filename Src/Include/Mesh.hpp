#include "Pch.hpp"

#include "Graphics/Buffer.hpp"

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
class Mesh
{
public:
	void Init(const wrl::ComPtr<ID3D11Device>& device, const char* filePath);
	void Draw(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);
	void UpdateTransformComponent(const wrl::ComPtr<ID3D11DeviceContext>& deviceContext);

public:
	VertexBuffer<Vertex> m_VertexBuffer;

	Transform m_Transform;
	ConstantBuffer<dx::XMMATRIX> m_TransformConstantBuffer;
};