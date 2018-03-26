#pragma once
#include <Core/StdIncludes.h>
#include <Core/Db.h>
#include "Tile.h"
#include "Models/Feature.h"

namespace DbInterface
{
	void CreateSaveGameDb(const char* const filename, bool create_test_data = false);
	std::vector<FeatureID> GetFeatureIDs(Db::Connection& conn, TileID tile_id);
	void PutFeature(Db::Connection& conn, Feature& feature);
	Feature GetFeature(Db::Connection& conn, FeatureID id);
}