#pragma once
#include <Core/StdIncludes.h>
#include "DualMesh.h"
#include "WidePoint.h"


class LandGenerator
{
	DualMesh _mesh;
public:
	LandGenerator(uint32_t seed);
	DualMesh& GetMesh() { return _mesh; }
};
