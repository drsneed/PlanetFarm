#include "Tile.h"
#include <cmath>

namespace
{
	// Masks to obtain the coordinates
	static uint32_t COORDINATE_MASKS[15] =
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
		0b11111111111111111111111111110000  // Zoom Level 14
	};
	// Mask to obtain the zoom level
	static uint32_t ZOOM_MASK = 0b00000000000000000000000000001111;

	static uint32_t MAX_KEY = 4294967295;
	static uint32_t MAX_ZOOM = 15;
}

auto Tile::ToBinaryQuadKey() -> uint32_t
{
	uint32_t key = 0;
	auto x32 = static_cast<uint32_t>(x);
	auto y32 = static_cast<uint32_t>(y);
	auto z32 = static_cast<uint32_t>(z);

	ASSERT(z32 < MAX_ZOOM);

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

Tile::Tile(uint32_t key)
{
	ASSERT(key < MAX_KEY);
	uint32_t x32 = 0;
	uint32_t y32 = 0;
	uint32_t z32 = key & ZOOM_MASK;

	ASSERT(z < MAX_ZOOM);
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

auto Tile::ToQuadKey() -> std::string
{
	auto binary_key = this->ToBinaryQuadKey();
	ASSERT(binary_key < MAX_KEY);
	auto z32 = binary_key & ZOOM_MASK;
	ASSERT(z32 < MAX_ZOOM);

	std::string result{};

	for(uint32_t i = 0; i < z32; ++i)
	{
		auto bit_location = 32 - ((i + 1) * 2);
		auto char_bits = ((binary_key & (0b11 << bit_location)) >> bit_location);
		result.push_back('0' + char_bits);
	}
	
	return result;
}

//pub fn parent_child_match(parent_key: u32, child_key : u32) -> bool {
//	debug_assert!(parent_key < MAX_KEY, "parent_key ({}) must be less than {}", parent_key, MAX_KEY);
//	debug_assert!(child_key < MAX_KEY, "child_key ({}) must be less than {}", child_key, MAX_KEY);
//	let parent_zoom = parent_key & ZOOM_MASK;
//	debug_assert!(parent_zoom < MAX_ZOOM, "parent_zoom ({}) must be less than {}", parent_zoom, MAX_ZOOM);
//	let child_zoom = child_key & ZOOM_MASK;
//	debug_assert!(child_zoom < MAX_ZOOM, "child_zoom ({}) must be less than {}", child_zoom, MAX_ZOOM);
//
//	if child_zoom <= parent_zoom{
//		return false;
//	}
//	let mask_index = parent_zoom as usize;
//	(parent_key & COORDINATE_MASKS[mask_index]) == (child_key & COORDINATE_MASKS[mask_index])
//}