#include "ModelsManager.h"

ModelsManager::ModelsManager()
{
}

Cube& ModelsManager::GetCube()
{
	return _cube;
}

DynamicFeature* ModelsManager::GetDynamicFeature(Feature* feature, uint8_t zoom_level)
{
	auto id = feature->GetID();

	if (_dynamic_features.count(id) == 0)
	{
		_dynamic_features[id] = DynamicFeature(feature, zoom_level);
	}
	
	return &_dynamic_features[id];
}