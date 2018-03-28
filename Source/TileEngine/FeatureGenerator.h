#pragma once
#include <Core/StdIncludes.h>
#include "Models/Feature.h"
#include "Tile.h"



class FeatureGenerator
{
	int _seed;
public:
	FeatureGenerator(int seed);
	Feature GenerateIsland();
};