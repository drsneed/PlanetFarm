#include "Map.h"
#include <cmath>
#include <TileEngine/Tile.h>

Map::Map()
	: _zoom_level(0)
	, _center((pow(2, TILE_MAX_ZOOM) * TILE_PIXEL_WIDTH) / 2.0f)
{
}

XMFLOAT3 Map::GetMouseCursorPosition(std::shared_ptr<Camera>& camera)
{
	XMFLOAT3 origin, direction;
	camera->ComputeRayFromMouseCursor(origin, direction);
	if (isnan(direction.x) || isnan(direction.y) || isnan(direction.z))
	{
		return XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
	XMVECTOR o = XMLoadFloat3(&origin);
	XMVECTOR d = XMLoadFloat3(&direction);

	auto bbox = BoundingBox
	{ 
		{_center, 0.0f, _center}, // Center of bbox
		{_center, 0.0f, _center} // Extents of bbox from center
	};

	float distance;
	if (bbox.Intersects(o, d, distance))
	{
		auto intersection_point = o + d * distance;
		XMFLOAT3 result;
		XMStoreFloat3(&result, intersection_point);
		return result;
	}
	else
	{
		return XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	
	

}
