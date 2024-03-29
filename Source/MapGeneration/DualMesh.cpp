#include "DualMesh.h"
#include <thinks/poissondisksampling.hpp>
#include "Triangulator.h"
#include <Core/DebugTools.h>
#include <sstream>
#include <set>


void DualMesh::_CheckTriangleInequality() 
{
	// check for skinny triangles
	auto badAngleLimit = 30;
	auto summary = std::vector<int>(badAngleLimit, 0);
	int count = 0;
	for (int s = 0; s < triangles.size(); s++) 
	{
		auto r0 = triangles[s],
			 r1 = triangles[Next(s)],
			 r2 = triangles[Next(Next(s))];
		auto p0 = vertices[r0],
			 p1 = vertices[r1],
			 p2 = vertices[r2];
		WidePoint d0 = { p0.x - p1.x, p0.y - p1.y };
		WidePoint d2 = { p2.x - p1.x, p2.y - p1.y };
		auto dotProduct = d0.x * d2.x + d0.y + d2.y;
		auto angleDegrees = 180.0 / static_cast<double>(XM_PI) * acos(dotProduct);
		if (angleDegrees < badAngleLimit) 
		{
			summary[static_cast<int>(floor(angleDegrees))]++;
			count++;
		}
	}
	// NOTE: a much faster test would be the ratio of the inradius to
	// the circumradius, but as I'm generating these offline, I'm not
	// worried about speed right now

	// TODO: consider adding circumcenters of skinny triangles to the point set
	//if (count > 0) 
	//{
	//	std::stringstream ss;
	//	ss << "[";
	//	for (int i = 0; i < summary.size(); ++i)
	//	{
	//		ss << "(" << i << ": " << summary[i] << ")";	
	//	}
	//	ss << "]";
	//	PRINTF(L"%S\n", ss.str().c_str());
	//}
}

void DualMesh::_CheckMeshConnectivity()
{
	// 1. make sure each side's opposite is back to itself
	// 2. make sure region-circulating starting from each side works
	auto ghost_r = vertices.size() - 1;
	std::vector<int> out_s;
	for (int s0 = 0; s0 < triangles.size(); s0++) 
	{
		if (half_edges[s0] == -1)
			continue;
		if (half_edges[half_edges[s0]] != s0) 
		{
			PRINTF(L"FAIL half_edges[half_edges[%d]] != %d\n", half_edges[half_edges[s0]], s0);
		}
		int s = s0;
		int count = 0;
		do {
			count++; 
			out_s.push_back(s);
			s = Next(half_edges[s]);
			if (count > 100 && triangles[s0] != ghost_r) 
			{
				std::stringstream ss;
				for (int i = 0; i < out_s.size(); ++i)
					ss << out_s[i] << " ";

				PRINTF(L"FAIL to circulate around region with start side = %d from region %d to %d, out_s = %S", 
					s0, triangles[s0], triangles[Next(s0)], ss.str().c_str());
				break;
			}

		} while (s != s0);
	}
}

void DualMesh::_AddGhostStructure()
{
	int numSolidSides = triangles.size();
	_ghost_index_tris = numSolidSides;
	int numVerts = vertices.size();
	_ghost_index_verts = numVerts;
	int numHalfEdges = half_edges.size();
	int numUnpairedSides = 0, firstUnpairedEdge = -1;
	std::map<int, int> unpaired;
	for (int s = 0; s < numSolidSides; s++)
	{
		if (half_edges[s] == -1)
		{
			numUnpairedSides++;
			unpaired[triangles[s]] = s;
			firstUnpairedEdge = s;
		}
	}
	const int ghost_start = vertices.size();
	vertices.push_back({ _max_bounds.x / 2.0, _max_bounds.y / 2.0 });
	triangles.resize(numSolidSides + 3 * numUnpairedSides);
	half_edges.resize(triangles.size());
	auto s = firstUnpairedEdge;
	for (int i = 0;
		i < numUnpairedSides;
		i++)
	{

		// Construct a ghost side for s
		int ghost_s = numSolidSides + 3 * i;
		half_edges[s] = ghost_s;
		half_edges[ghost_s] = s;
		auto index = Next(s);
		triangles[ghost_s] = triangles[index];

		// Construct the rest of the ghost triangle
		triangles[ghost_s + 1] = triangles[s];
		triangles[ghost_s + 2] = ghost_start;
		auto k = numSolidSides + (3 * i + 4) % (3 * numUnpairedSides);
		half_edges[ghost_s + 2] = k;
		half_edges[k] = ghost_s + 2;
		auto next_s = Next(s);
		//TODO: 76th entry should be 2. It is 68.
		auto tri = triangles[next_s];
		auto unpaired_half_edge = unpaired[tri];
		s = unpaired[triangles[Next(s)]];
	}
}

DualMesh::DualMesh(uint32_t seed, const WidePoint& max_bounds, const double point_spacing)
	: _max_bounds(max_bounds)
{
	auto width = max_bounds.x;
	const int n = ceil(width / point_spacing);
	_num_boundary_regions = (n + 1) * 4;
	vertices.resize(_num_boundary_regions);
	for (int i = 0; i <= n; i++)
	{
		auto t = (i + 0.5) / (n + 1.0);
		auto w = width * t;
		auto offset = pow(t - 0.5, 2);
		vertices[4 * i] = WidePoint{ offset, w };
		vertices[4 * i + 1] = WidePoint{ width - offset, w };
		vertices[4 * i + 2] = WidePoint{ w, offset };
		vertices[4 * i + 3] = WidePoint{ w, width - offset };
	}
	
	thinks::poissonDiskSampling(vertices, point_spacing, { 0.0, 0.0 }, max_bounds, 30, seed);

	

	Triangulator triangulator(vertices);
	triangles = triangulator.GetTriangles();
	half_edges = triangulator.GetHalfEdges();
	auto tri_count = triangles.size();
	//PRINTF(L"VERTS\n---------------------\n");
	//for (int i = 0; i < vertices.size(); ++i)
	//{
	//	PRINTF(L"%d. (%f, %f)\n", i, vertices[i].x, vertices[i].y);
	//}
	//PRINTF(L"TRIS\n---------------------\n");
	//for (int i = 0; i < triangles.size(); ++i)
	//{
	//	PRINTF(L"%d. (%d)\n", i, triangles[i]);
	//}
	//PRINTF(L"EDGES\n---------------------\n");
	//for (int i = 0; i < half_edges.size(); ++i)
	//{
	//	PRINTF(L"%d. (%d)\n", i, half_edges[i]);
	//}

	//checkPointInequality(graph);
	_CheckTriangleInequality();
	_AddGhostStructure();
	

	//_CheckMeshConnectivity();

	regions = std::vector<int>(vertices.size(), 0);
	for (int s = 0; s < triangles.size(); s++) 
	{
		if (regions[triangles[s]] == 0)
			regions[triangles[s]] = s;
	}

	region_vertices = std::vector<WidePoint>(triangles.size() / 3);
	for (auto s = 0; s < triangles.size(); s += 3) 
	{
		WidePoint a = vertices[triangles[s]];
		WidePoint b = vertices[triangles[s + 1]];
		WidePoint c = vertices[triangles[s + 2]];
		if (s >= _ghost_index_tris) 
		{
			// ghost triangle center is just outside the unpaired side
			auto dx = b.x - a.x;
			auto dy = b.y - a.y;
			region_vertices[s / 3] = { a.x + 0.5*(dx + dy), a.y + 0.5*(dy - dx) };
		}
		else 
		{
			// solid triangle center is at the centroid
			region_vertices[s / 3] = { (a.x + b.x + c.x) / 3.0,
				(a.y + b.y + c.y) / 3.0 };
		}
	}
}

bool DualMesh::IsBoundaryRegion(int region_index)
{
	// easy check since we put all the boundary regions at the beginning of the vertex array
	return region_index < _num_boundary_regions;
}

std::vector<int> DualMesh::GetRegionNeighbors(int region_index)
{
	std::vector<int> output;
	const int s0 = regions[region_index];
	auto s = s0;
	do
	{
		output.push_back(triangles[Next(s)]);
		s = Next(half_edges[s]);
	} while (s != s0);

	return output;
}

std::vector<int> DualMesh::GetRegionVertexNeighbors(int t_index)
{
	//t_circulate_t(out_t, t) { out_t.length = 3; for (let i = 0; i < 3; i++) { out_t[i] = this.s_outer_t(3*t+i); } return out_t; }
	std::vector<int> output;
	for (int i = 0; i < 3; ++i)
	{
		output.push_back(s_to_t(half_edges[(3 * t_index + i)]));
	}

	return output;

}

void DualMesh::GetFlankingRegions(int t0, int t1, int& r1, int& r2)
{
	std::set<int> t0_regions;
	std::set<int> t1_regions;

	for (int i = 0; i < 3; i++) 
	{ 
		t0_regions.insert(triangles[3 * t0 + i]);
		t1_regions.insert(triangles[3 * t1 + i]);
	}

	std::vector<int> common_regions;
	std::set_intersection(t0_regions.begin(), t0_regions.end(),
		t1_regions.begin(), t1_regions.end(),
		std::inserter(common_regions, common_regions.begin()));
	if (common_regions.size() == 2)
	{
		r1 = common_regions[0];
		r2 = common_regions[1];
	}
	else
	{
		r1 = -1;
		r2 = -1;
	}
}

std::vector<int> DualMesh::GetRegionEdges(int region_index)
{
	std::vector<int> output;
	const int s0 = regions[region_index];
	auto s = s0;
	do
	{
		output.push_back(s);
		s = Next(half_edges[s]);
	} while (s != s0);

	return output;

}

std::vector<WidePoint> DualMesh::GetRegionVertices(int region_index)
{
	std::vector<WidePoint> output;
	const int s0 = regions[region_index];
	auto s = s0;
	do 
	{
		output.push_back(region_vertices[s_to_t(s)]);
		s = Next(half_edges[s]);
	} while (s != s0);

	return output;
	
}

std::vector<int> DualMesh::GetRegionVerticesI(int region_index)
{
	std::vector<int> output;
	const int s0 = regions[region_index];
	auto s = s0;
	do
	{
		output.push_back(s_to_t(s));
		s = Next(half_edges[s]);
	} while (s != s0);

	return output;

}

