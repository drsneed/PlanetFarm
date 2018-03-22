#pragma once
#include <Core/StdIncludes.h>
#include <Core/Db.h>
#include "Tile.h"
#include "Resource.h"

namespace DbInterface
{
	void CreateSaveGameDb(const char* const filename, bool create_test_data = false);
	std::vector<ResourceID> QueryResourceIDs(Db::Connection& conn, TileID tile_id);
	void PutResource(Db::Connection& conn, Resource& resource);
	bool GetResource(Db::Connection& conn, TileID tile_id, ResourceID resource_id, Resource& out);
	bool GetResource(Db::Connection& conn, int64_t row_id, Resource& out);
}