#pragma once
#include <Game/Models/Cube.h>
#include "Models/Feature.h"
#include <map>

class ModelsManager
{
	Cube _cube;
	std::map<FeatureID, DynamicFeature> _dynamic_features;
public:
	ModelsManager();
	Cube& GetCube();

	DynamicFeatureView GetDynamicFeatureView(Feature* feature, TileID tile_id);

};