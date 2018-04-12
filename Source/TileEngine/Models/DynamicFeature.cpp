#include "Feature.h"
#include "DynamicFeature.h"

DynamicFeature::View::View(const std::vector<XMFLOAT2>& vertex_data)
	: vertex_buffer(nullptr)
	, vertex_count(0)
{
	CreateBuffers(vertex_data);
}

void DynamicFeature::View::CreateBuffers(const std::vector<XMFLOAT2>& vertex_data)
{
	ASSERT(vertex_buffer == nullptr);
	ASSERT(vertex_count == 0);

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.ByteWidth = sizeof(XMFLOAT2) * vertex_data.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexSubresource;
	ZeroMemory(&vertexSubresource, sizeof(vertexSubresource));
	vertexSubresource.pSysMem = &vertex_data[0];

	auto device = GraphicsWindow::GetInstance()->GetDevice();

	if (!D3DCheck(device->CreateBuffer(&vertexBufferDesc, &vertexSubresource, &vertex_buffer),
		L"ID3D11Device::CreateBuffer (DynamicFeature::View, VertexBuffer)")) return;
	vertex_count = vertex_data.size();
}

DynamicFeature::DynamicFeature()
	: color(0)
	, rotation(0.0f)
	, tile_id(INVALID_TILE_ID)
{
}

DynamicFeature::DynamicFeature(Feature* feature, uint8_t zoom_level)
	: position(feature->GetMapPosition())
	, color(0x33FF33FF)
	, rotation(0.0f)
	, tile_id(feature->GetTileID())
{
	ASSERT(feature->HasPoints());
	_views[feature->GetTileID()] = View(feature->GetPointsRef());
	
}

DynamicFeature::~DynamicFeature()
{
}

auto DynamicFeature::GetView(TileID tile_id) -> View*
{
	std::lock_guard<std::mutex> guard(_views_mutex);

	if (tile_id == this->tile_id || _views.count(tile_id) == 0)
	{
		ASSERT(this->tile_id != INVALID_TILE_ID);
		return &_views[this->tile_id];
	}

	return &_views[tile_id];
	
}