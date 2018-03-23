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
	Resource GetResource(Db::Connection& conn, ResourceID id);
}