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

	auto left = static_cast<int>(floor(top_left.x / TILE_PIXEL_WIDTH)) - offset;
	auto top = static_cast<int>(floor(top_left.y / TILE_PIXEL_WIDTH)) - offset;
	auto right = static_cast<int>(floor(bottom_right.x / TILE_PIXEL_WIDTH)) - offset;
	auto bottom = static_cast<int>(floor(bottom_right.y / TILE_PIXEL_WIDTH)) - offset;

    



	std::vector<Tile> result/*(((right - left) + 1) * ((top - bottom) + 1))*/;
	size_t count = 0;
	for (int x = left; x <= right; ++x)
	{
		for (int y = bottom; y <= top; ++y)
		{
			if (x < 0 || y < 0)
				continue;
			Tile tile(x, y, zoom_level);
			if (tile.IsValid())
			{
				result.push_back(tile);
			}
		}
	}
	
	return result; 
}

bool TileEngine::GetTileContaining(XMFLOAT2 map_point, Tile& tile)
{
	if (tile.z != TILE_MAX_ZOOM)
	{
		map_point = Tile::TransformCoordinates(map_point, TILE_MAX_ZOOM, tile.z);
	}

	tile.x = static_cast<uint16_t>(floor(map_point.x / TILE_PIXEL_WIDTH));
	tile.y = static_cast<uint16_t>(floor(map_point.y / TILE_PIXEL_WIDTH));

	return tile.IsValid();
}