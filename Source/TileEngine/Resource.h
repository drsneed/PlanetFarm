#pragma once
#include <Core/StdIncludes.h>
#include "Tile.h"
#include <cstdlib>

typedef int64_t ResourceID;

enum class ResourceType : uint16_t
{
	Empty,
	ResidentialBuilding1,
	ResidentialBuilding2,
	CommericalBuilding1,
	IndustrialBuilding1
};

struct Payload
{
	size_t blob_size;
	void* blob;

	Payload() : blob_size(0), blob(nullptr) {}

	Payload(const void* db_blob, size_t db_blob_size)
		: blob_size(0), blob(nullptr)
	{
		ASSERT(db_blob_size > 0);
		blob = ::malloc(db_blob_size);
		if (blob)
		{
			blob_size = db_blob_size;
			memcpy_s(blob, blob_size, db_blob, db_blob_size);
		}
	}
	~Payload()
	{
		if (blob)
		{
			::free(blob);
		}

	}

	// no copying
	Payload(Payload const&) = delete;
	Payload& operator=(Payload const&) = delete;

	// move ok
	Payload(Payload&& other) noexcept
		: blob(other.blob)
		, blob_size(other.blob_size)
	{ 
		other.blob = nullptr;
		other.blob_size = 0;
	}

	inline Payload& operator=(Payload&& other) noexcept
	{
		if (this == &other)
			return *this;
		blob = other.blob;
		blob_size = other.blob_size;
		other.blob = nullptr;
		other.blob_size = 0;
		return *this;
	}
};

struct Resource
{
	ResourceID id;
	TileID tile_id;
	ResourceType type;
	Payload payload;

	Resource()
		: id(0)
		, tile_id(0)
		, type (ResourceType::Empty)
		, payload()
	{}

	Resource(ResourceID id, TileID tile_id, ResourceType type, void* payload_blob, size_t payload_blob_size)
		: id(id)
		, tile_id(tile_id)
		, type(type)
		, payload(payload_blob, payload_blob_size)
	{}

	// no copying
	Resource(Resource const&) = delete;
	Resource& operator=(Resource const&) = delete;

	// move ok
	Resource(Resource&& other) noexcept
		: id(other.id)
		, tile_id(other.tile_id)
		, type(other.type)
		, payload(std::move(other.payload))
	{
		other.id = 0;
		other.tile_id = 0;
		other.type = ResourceType::Empty;
		other.payload.blob = nullptr;
		other.payload.blob_size = 0;
	}

	inline Resource& operator=(Resource&& other) noexcept
	{
		if (this == &other)
			return *this;

		id = other.id;
		tile_id = other.tile_id;
		type = other.type;
		payload = std::move(other.payload);

		other.id = 0;
		other.tile_id = 0;
		other.type = ResourceType::Empty;
		other.payload.blob = nullptr;
		other.payload.blob_size = 0;
		return *this;
	}
};