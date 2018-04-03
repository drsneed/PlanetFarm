#pragma once
#include <Core/StdIncludes.h>
#include "WidePoint.h"

struct TempLand
{
	std::vector<int> tris;
	std::vector<WidePoint> vertices;
};


class LandGenerator
{
public:
	LandGenerator();
	TempLand GetLand(uint32_t seed);
};
