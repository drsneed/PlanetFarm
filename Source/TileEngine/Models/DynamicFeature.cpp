#include "Feature.h"
#include "DynamicFeature.h"

DynamicFeature::View::View(DynamicFeature* parent, const std::vector<XMFLOAT2>& vertex_data)
	: vertex_buffer(nullptr)
	, parent(parent)
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
	, tile_id(INVALID_TILE_ID)
{
}

DynamicFeature::DynamicFeature(Feature* feature)
	: position(feature->GetMapPosition())
	, color(0x33FF33FF)
	, tile_id(feature->GetTileID())
{
	ASSERT(feature->IsDynamic());
	_views[feature->GetTileID()] = View(this, feature->GetPointsRef());
	
}

DynamicFeature::~DynamicFeature()
{
	for (auto& view : _views)
	{
		if(view.second.vertex_buffer)
			view.second.vertex_buffer->Release();
	}
}

//TileID DynamicFeature::_FindViewRecursive(TileID good_)
//{
//	if (_views.count(tile_id) == 1)
//	{
//
//	}
//}

auto DynamicFeature::GetView(TileID requested_tile_id) -> View
{

	//std::lock_guard<std::mutex> guard(_views_mutex);
	//There is guaranteed to be at least one view of this feature, it's own tile.
	//If the requested tile
	if (_views.count(requested_tile_id) == 1)
	{
		return _views[requested_tile_id];
	}
	else
	{
		//TODO: Build view of a subset of the data for this tile
		return _views[tile_id];
	}
}