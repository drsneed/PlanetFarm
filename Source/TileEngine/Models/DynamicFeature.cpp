#include "Feature.h"
#include "DynamicFeature.h"



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
	_views[feature->GetTileID()] = DynamicFeatureView(this, feature->GetPointsRef());
	
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

void DynamicFeature::_BuildView(TileID tile_id)
{

}

auto DynamicFeature::GetView(TileID requested_tile_id) -> DynamicFeatureView
{

	//std::lock_guard<std::mutex> guard(_views_mutex);
	//There is guaranteed to be at least one view of this feature, it's own tile.
	//If the requested tile
	if (_views.count(requested_tile_id) == 0)
		_BuildView(requested_tile_id);
	return _views[requested_tile_id];
}