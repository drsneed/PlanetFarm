#include "TileEngine.h"
#include <Core/GraphicsWindow.h>

std::vector<Tile> TileEngine::Fetch(BoundingRect viewable_area, uint8_t zoom_level)
{
	XMFLOAT2 top_left, bottom_right;
	viewable_area.GetCorners(top_left, bottom_right);

	float max_index = pow(2, zoom_level) - 1;

	uint16_t left = static_cast<uint16_t>(max(floor(top_left.x / TILE_PIXEL_WIDTH), 0.0f));
	uint16_t top = static_cast<uint16_t>(min(floor(top_left.y / TILE_PIXEL_WIDTH), max_index));
	uint16_t right = static_cast<uint16_t>(min(floor(bottom_right.x / TILE_PIXEL_WIDTH), max_index));
	uint16_t bottom = static_cast<uint16_t>(max(floor(bottom_right.y / TILE_PIXEL_WIDTH), 0.0f));

	// bounds check the viewable_area with this zoom level



	std::vector<Tile> result(((right - left) + 1) * ((top - bottom) + 1));
	size_t index = 0;

	for (uint16_t x = left; x <= right; ++x)
	{
		for (uint16_t y = bottom; y <= top; ++y)
		{
			result[index].x = x;
			result[index].y = y;
			result[index].z = zoom_level;
			index++;
		}
	}

	return result; 
}

bool TileEngine::GetTileContaining(const XMFLOAT2& map_point, Tile& tile)
{
	auto level_width = (pow(2, TILE_MAX_ZOOM)) * TILE_PIXEL_WIDTH;
	if (tile.z == TILE_MAX_ZOOM)
	{
		tile.x = floor(map_point.x / TILE_PIXEL_WIDTH);
		tile.y = floor(map_point.y / TILE_PIXEL_WIDTH);
		return true;
	}

	auto level_0_width = 
	auto ix = floor(x / TILE_PIXEL_WIDTH);
	auto iy = floor(y / TILE_PIXEL_WIDTH);
}