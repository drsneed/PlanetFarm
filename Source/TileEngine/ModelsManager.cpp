#include "ModelsManager.h"

ModelsManager::ModelsManager()
{
}

Cube& ModelsManager::GetCube()
{
	return _cube;
}

DynamicFeature::View* ModelsManager::GetDynamicFeatureView(Feature* feature, TileID tile_id)
{
	auto id = feature->GetID();

	if (_dynamic_features.count(id) == 0)
	{
		_dynamic_features[id] = DynamicFeature(feature);
	}
	
	return _dynamic_features[id].GetView(tile_id);
}