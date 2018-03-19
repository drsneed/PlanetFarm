#pragma once

#include <Core/StdIncludes.h>
#include "Tile.h"
#include "BoundingRect.h"

class TileEngine
{
	
public:
	std::vector<Tile> Fetch(BoundingRect viewable_area, uint8_t zoom_level);
	bool GetTileContaining(XMFLOAT2 map_point, Tile& tile);
};