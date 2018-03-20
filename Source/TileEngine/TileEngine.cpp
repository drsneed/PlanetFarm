#include "TileEngine.h"
#include <Core/GraphicsWindow.h>

std::vector<Tile> TileEngine::Fetch(BoundingRect viewable_area, uint8_t zoom_level)
{
	XMFLOAT2 top_left, bottom_right;
	viewable_area.GetCorners(top_left, bottom_right);
	int offset = (TILE_SPAN_MAX - TILE_SPAN[zoom_level]) / 2;
	auto left = max(static_cast<int>(floor(top_left.x / TILE_PIXEL_WIDTH)) - offset, 0);
	auto top = min(static_cast<int>(floor(top_left.y / TILE_PIXEL_WIDTH)) - offset, TILE_SPAN[zoom_level] - 1);
	auto right = min(static_cast<int>(floor(bottom_right.x / TILE_PIXEL_WIDTH)) - offset, TILE_SPAN[zoom_level] - 1);
	auto bottom = max(static_cast<int>(floor(bottom_right.y / TILE_PIXEL_WIDTH)) - offset, 0);
	std::vector<Tile> result;
	for (int x = left; x <= right; ++x)
		for (int y = bottom; y <= top; ++y)
			result.push_back(Tile(x, y, zoom_level));
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