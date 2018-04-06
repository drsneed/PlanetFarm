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
	int _ghost_index_verts;
	int _ghost_index_tris;
	int _num_boundary_regions;
	std::vector<int> _regions;

public:
	std::vector<WidePoint> vertices;
	std::vector<int> triangles;
	std::vector<int> half_edges;
	std::vector<WidePoint> region_vertices;
	
	std::vector<WidePoint> GetRegionVertices(int region_index);
	std::vector<int> GetRegionNeighbors(int region_index);
	int GetGhostIndexVerts() { return _ghost_index_verts; }
	int GetGhostIndexTris() { return _ghost_index_tris; }
	int GetRegionCount() { return vertices.size();  }
	bool IsBoundaryRegion(int region_index);
	WidePoint Center() { return { _max_bounds.x / 2.0, _max_bounds.y / 2.0 }; }
	DualMesh(uint32_t seed, const WidePoint& max_bounds, const double point_spacing = 2.0);
};