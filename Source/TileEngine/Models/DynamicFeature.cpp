#include "Feature.h"
#include "DynamicFeature.h"

DynamicFeature::DynamicFeature(Feature* feature, uint8_t zoom_level)
	: vertex_buffer(nullptr)
	, vertex_count(feature->GetPointsRef().size())
	, position(feature->GetMapPosition())
	, color(0x33FF33FF)
	, rotation(0.0f)
	, scale(1.0f * ((int)zoom_level - (int)(Tile(feature->GetTileID()).z) + 1))
{
	ASSERT(feature->HasPoints());
	auto& point_data = feature->GetPointsRef();
	
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.ByteWidth = sizeof(XMFLOAT2) * point_data.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexSubresource;
	ZeroMemory(&vertexSubresource, sizeof(vertexSubresource));
	vertexSubresource.pSysMem = &point_data[0];

	auto device = GraphicsWindow::GetInstance()->GetDevice();

	if (!D3DCheck(device->CreateBuffer(&vertexBufferDesc, &vertexSubresource, &vertex_buffer),
		L"ID3D11Device::CreateBuffer (DynamicFeature, VertexBuffer)")) return;
}

DynamicFeature::~DynamicFeature()
{
	if (vertex_buffer)
	{
		vertex_buffer->Release();
	}
}