#pragma once
#include <Core/StdIncludes.h>
#include <Core/DebugTools.h>
#include <cstdint>
// Tile coordinates are stored as byte-encoded quad keys.
// The key is stored as a 32 bit unsigned integer.
// Each coordinate gets 14 bits of space and the zoom level gets 4 bits.
// Therefore, the maximum coordinate value is 2^14(16384) minus one (zero-based)
// The zoom level is stored in the 4 least significant bits. 
// This allows a maximum of 16 zoom levels (0 thru 15), however with only 14 bits per coordinate, 
// We cannot store a higher tile address than (16383, 16383, zoom 13)

#define TILE_MAX_ZOOM 14
#define TILE_PIXEL_WIDTH 256
#define TILE_PIXEL_WIDTH_HALF 128.0f

enum class TileDataType : uint8_t
{
	Point,
	Line,
	Polygon
};

struct TileData
{
	TileDataType data_type;
	std::vector<XMFLOAT2> data;
};

struct Tile
{
	Tile();
	Tile(const Tile& tile)
	{
		x = tile.x;
		y = tile.y;
		z = tile.z;
	}
	Tile(uint16_t x, uint16_t y, uint8_t z)
		: data(nullptr), x(x), y(y), z(z) {}
	Tile(uint32_t key);
	auto ToBinaryQuadKey() -> uint32_t;
	auto ToQuadKey() -> std::string;

	auto GetPosition() const -> XMFLOAT2;
	auto IsValid() const -> bool;
	auto Contains(XMFLOAT2 map_point) -> bool;
	
	std::unique_ptr<TileData> data;

	uint16_t x;
	uint16_t y;
	uint8_t z;

	static auto IsParentChildRelation(uint32_t parent_key, uint32_t child_key) -> bool;
	static auto GetLevelWidth(uint8_t zoom_level) -> float;
};

void RunTileTest();