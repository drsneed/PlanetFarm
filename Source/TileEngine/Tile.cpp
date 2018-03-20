#include "Tile.h"
#include <cmath>

int TILE_SPAN[TILE_MAX_ZOOM + 1] =
{
	1,
	2,
	4,
	8,
	16,
	32,
	64,
	128,
	256,
	512,
	1024,
	2048,
	4096,
	8192,
	16384
};

namespace
{
	// Masks to obtain the coordinates
	static uint32_t COORDINATE_MASKS[TILE_MAX_ZOOM + 2] =
	{
		0b00000000000000000000000000000000, // Zoom Level 0
		0b11000000000000000000000000000000, // Zoom Level 1
		0b11110000000000000000000000000000, // Zoom Level 2
		0b11111100000000000000000000000000, // Zoom Level 3
		0b11111111000000000000000000000000, // Zoom Level 4
		0b11111111110000000000000000000000, // Zoom Level 5
		0b11111111111100000000000000000000, // Zoom Level 6
		0b11111111111111000000000000000000, // Zoom Level 7
		0b11111111111111110000000000000000, // Zoom Level 8
		0b11111111111111111100000000000000, // Zoom Level 9
		0b11111111111111111111000000000000, // Zoom Level 10
		0b11111111111111111111110000000000, // Zoom Level 11
		0b11111111111111111111111100000000, // Zoom Level 12
		0b11111111111111111111111111000000, // Zoom Level 13
		0b11111111111111111111111111110000, // Zoom Level 14
		0b00000000000000000000000000001111, // Zoom Mask (or invalid tile: (x: 0, y:0, z:15))
	};
	#define ZOOM_MASK COORDINATE_MASKS[TILE_MAX_ZOOM + 1]

	static uint32_t MAX_KEY = 4294967295;
}

auto Tile::GetID() -> uint32_t
{
	uint32_t key = 0;
	auto x32 = static_cast<uint32_t>(x);
	auto y32 = static_cast<uint32_t>(y);
	auto z32 = static_cast<uint32_t>(z);

	ASSERT(z32 < TILE_MAX_ZOOM);

	auto max_coord = pow(2, z32);

	ASSERT(x32 < max_coord);
	ASSERT(y32 < max_coord);

	for (uint32_t i = z32; i > 0; --i)
	{
		auto mask = 1 << (i - 1);
		auto bit_location = 32 - ((z32 - i + 1) * 2) + 1;
		if(x32 & mask)
			key |= 1 << (bit_location - 1);
		if(y32 & mask)
			key |= 1 << bit_location;
	}
	return (key | z32);
}

Tile::Tile()
	: x(0)
	, y(0)
	, z(0)
{

}

Tile::Tile(uint32_t key)
{
	ASSERT(key < MAX_KEY);
	uint32_t x32 = 0;
	uint32_t y32 = 0;
	uint32_t z32 = key & ZOOM_MASK;

	ASSERT(z < TILE_MAX_ZOOM);
	for (uint32_t i = 0; i < z32; ++i)
	{
		auto bit_location = 32 - ((i + 1) * 2);
		auto char_bits = (key & (0b11 << bit_location)) >> bit_location;
		auto mask = 1 << (z32 - i - 1);

		if (char_bits == 1)
			x32 |= mask;
		else if (char_bits == 2)
			y32 |= mask;
		else if (char_bits == 3)
		{
			x32 |= mask;
			y32 |= mask;
		}
	}
	x = static_cast<uint16_t>(x32);
	y = static_cast<uint16_t>(y32);
	z = static_cast<uint8_t>(z32);
}

auto Tile::Contains(XMFLOAT2 map_point) -> bool
{
	int level_0_span = pow(2, TILE_MAX_ZOOM);
	int this_level_span = pow(2, z);
	int offset = (level_0_span - this_level_span) / 2;
	auto level_0_x = static_cast<int>(floor(map_point.x / TILE_PIXEL_WIDTH));
	auto level_0_y = static_cast<int>(floor(map_point.y / TILE_PIXEL_WIDTH));
	auto mpx = level_0_x - offset;
	auto mpy = level_0_y - offset;
	return mpx == static_cast<int>(x) && mpy == static_cast<int>(y);
}

auto Tile::IsValid() const -> bool
{
	auto index_limit = pow(2, z);
	return x < index_limit && y < index_limit;
}

auto Tile::GetPosition() const -> XMFLOAT2
{
	float index_offset = (TILE_SPAN_MAX - TILE_SPAN[z]) / 2.0f;

	auto fx = static_cast<float>(x) + index_offset;
	auto fy = static_cast<float>(y) + index_offset;
	return XMFLOAT2 { (fx * TILE_PIXEL_WIDTH) + TILE_PIXEL_WIDTH_HALF, 
		(fy * TILE_PIXEL_WIDTH) + TILE_PIXEL_WIDTH_HALF };
	
}

auto Tile::GetLevelWidth(uint8_t zoom_level) -> float
{
	return pow(2, zoom_level) * TILE_PIXEL_WIDTH;
}

auto Tile::GetQuadKey() -> std::string
{
	auto binary_key = this->GetID();
	ASSERT(binary_key < MAX_KEY);
	auto z32 = binary_key & ZOOM_MASK;
	ASSERT(z32 < TILE_MAX_ZOOM);

	std::string result{};

	for(uint32_t i = 0; i < z32; ++i)
	{
		auto bit_location = 32 - ((i + 1) * 2);
		auto char_bits = ((binary_key & (0b11 << bit_location)) >> bit_location);
		result.push_back('0' + char_bits);
	}
	
	return result;
}

auto Tile::IsParentChildRelation(uint32_t parent_key, uint32_t child_key) -> bool
{
	auto parent_zoom = parent_key & ZOOM_MASK;
	auto child_zoom = child_key & ZOOM_MASK;

	ASSERT(parent_key < MAX_KEY);
	ASSERT(child_key < MAX_KEY);
	ASSERT(parent_zoom < TILE_MAX_ZOOM);
	ASSERT(child_zoom < TILE_MAX_ZOOM);
	
	if(child_zoom <= parent_zoom)
		return false;
	return (parent_key & COORDINATE_MASKS[parent_zoom]) == (child_key & COORDINATE_MASKS[parent_zoom]);
}

void RunTileTest()
{
	auto parent_tile = Tile(25, 43, 6);
	auto child_tile = Tile(51, 87, 7);
	auto parent_qk = parent_tile.GetQuadKey();
	auto child_qk = child_tile.GetQuadKey();
	char buffer[256];

	if (Tile::IsParentChildRelation(parent_tile.GetID(), child_tile.GetID()))
	{
		wsprintfA(buffer, "%s is the parent of %s\n", parent_qk.c_str(), child_qk.c_str());
		OutputDebugStringA(buffer);
	}
	else
	{
		wsprintfA(buffer, "%s is NOT the parent of %s\n", parent_qk.c_str(), child_qk.c_str());
		OutputDebugStringA(buffer);
	}

	auto new_tile = Tile(16383, 16383, 14);
	auto quadkey = new_tile.GetQuadKey();
	OutputDebugStringA(quadkey.c_str());
}