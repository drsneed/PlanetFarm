#pragma once
#include <Core/StdIncludes.h>
#include "Tile.h"
#include <cstdlib>

typedef uint32_t ResourceID;

struct Payload
{
	size_t blob_size;
	void* blob;

	Payload() : blob_size(0), blob(nullptr) {}

	Payload(const void* db_blob, size_t db_blob_size)
		: blob_size(0), blob(nullptr)
	{
		ASSERT(db_blob_size > 0);
		blob = malloc(db_blob_size);
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
			free(blob);
			blob = nullptr;
		}

	}

	// no copying
	Payload(Payload const&) = delete;
	Payload& operator=(Payload const&) = delete;

	// move ok
	Payload(Payload&& other) noexcept
		: blob(std::move(other.blob))
		, blob_size(other.blob_size)
	{ 

	}

	inline Payload& operator=(Payload&& other) noexcept
	{
		if (this == &other)
			return *this;
		blob = std::move(other.blob);
		blob_size = other.blob_size;
		return *this;
	}
};

struct Resource
{
	int64_t row_id;
	TileID tile_id;
	ResourceID resource_id;
	Payload payload;
};