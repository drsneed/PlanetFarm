#pragma once

#include <Core/StdIncludes.h>
#include "Tile.h"

class TileEngine
{
	
public:
	std::vector<Tile> Fetch(BoundingBox viewport, uint8_t zoom_level);
};