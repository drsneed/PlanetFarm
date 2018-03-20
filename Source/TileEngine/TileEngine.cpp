#include "TileEngine.h"
#include <Core/GraphicsWindow.h>

std::vector<Tile> TileEngine::Fetch(BoundingRect viewable_area, uint8_t zoom_level)
{
	XMFLOAT2 top_left, bottom_right;
	viewable_area.GetCorners(top_left, bottom_right);

	//if (zoom_level != TILE_MAX_ZOOM)
	//{
	//	top_left = Tile::TransformCoordinates(top_left, TILE_MAX_ZOOM, zoom_level);
	//	bottom_right = Tile::TransformCoordinates(bottom_right, TILE_MAX_ZOOM, zoom_level);
	//}

	float max_index = pow(2, zoom_level) - 1;
	int level_0_span = pow(2, TILE_MAX_ZOOM);
	int this_level_span = pow(2, zoom_level);
	int offset = (level_0_span - this_level_span) / 2;

	auto left = max(static_cast<int>(floor(top_left.x / TILE_PIXEL_WIDTH)) - offset, 0);
	auto top = min(static_cast<int>(floor(top_left.y / TILE_PIXEL_WIDTH)) - offset, this_level_span - 1);
	auto right = min(static_cast<int>(floor(bottom_right.x / TILE_PIXEL_WIDTH)) - offset, this_level_span - 1);
	auto bottom = max(static_cast<int>(floor(bottom_right.y / TILE_PIXEL_WIDTH)) - offset, 0);

    
	std::vector<Tile> result(((right - left) + 1) * ((top - bottom) + 1));
	size_t count = 0;
	for (int x = left; x <= right; ++x)
	{
		for (int y = bottom; y <= top; ++y)
		{
			result[count].x = x;
			result[count].y = y;
			result[count].z = zoom_level;
			count++;
		}
	}
	
	return result; 
}

bool TileEngine::GetTileContaining(XMFLOAT2 map_point, Tile& tile)
{
	int level_0_span = pow(2, TILE_MAX_ZOOM);
	int this_level_span = pow(2, tile.z);
	int offset = (level_0_span - this_level_span) / 2;

	auto x = static_cast<int>(floor(map_point.x / TILE_PIXEL_WIDTH)) - offset;
	auto y = static_cast<int>(floor(map_point.y / TILE_PIXEL_WIDTH)) - offset;

	if (x >= 0 && x < this_level_span && y >= 0 && y < this_level_span)
	{
		tile.x = static_cast<uint16_t>(x);
		tile.y = static_cast<uint16_t>(y);
		return true;
	}

	return false;
}