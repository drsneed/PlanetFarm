#pragma once
#include <Core/StdIncludes.h>

struct TempLand
{
	std::vector<int> tris;
	std::vector<MapPoint> vertices;
};


class LandGenerator
{
public:
	LandGenerator();
	TempLand GetLand();
};
