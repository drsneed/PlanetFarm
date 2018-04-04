#pragma once
#include <Core/StdIncludes.h>
#include "WidePoint.h"

static int s_to_t(int s) { return (s / 3) | 0; }
static int Previous(int s) { return (s % 3 == 0) ? s + 2 : s - 1; }
static int Next(int s) { return (s % 3 == 2) ? s - 2 : s + 1; }

class DualMesh
{
	void _CheckTriangleInequality();
	void _CheckMeshConnectivity();
	void _AddGhostStructure();
	WidePoint _max_bounds;
	int _num_boundary_regions;
public:
	std::vector<WidePoint> vertices;
	std::vector<int> triangles;
	std::vector<int> half_edges;
	DualMesh(uint32_t seed, const WidePoint& max_bounds, const double point_spacing = 2.0);
};