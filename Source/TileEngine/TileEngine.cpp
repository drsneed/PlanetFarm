#include "TileEngine.h"
#include <Core/GraphicsWindow.h>

std::vector<Tile> TileEngine::Fetch(BoundingRect viewable_area, uint8_t zoom_level)
{
	XMFLOAT2 top_left, bottom_right;
	viewable_area.GetCorners(top_left, bottom_right);

	uint16_t left = static_cast<uint16_t>(floor(top_left.x / TILE_PIXEL_WIDTH));
	uint16_t top = static_cast<uint16_t>(floor(top_left.y / TILE_PIXEL_WIDTH));
	uint16_t right = static_cast<uint16_t>(floor(bottom_right.x / TILE_PIXEL_WIDTH));
	uint16_t bottom = static_cast<uint16_t>(floor(bottom_right.y / TILE_PIXEL_WIDTH));

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