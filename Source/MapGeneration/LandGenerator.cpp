#include "LandGenerator.h"
#include "Triangulator.h"
#include <Core/DebugTools.h>
#include <set>
#include <stack>


//TODO: New plan to trace coastlines:
//During map generation, build index where you can input a vertex id and it returns the nex
namespace
{
	inline double mix(double a, double b, double t) 
	{
		return a * (1.0 - t) + b * t;
	};

	inline WidePoint mixp(const WidePoint& p, const WidePoint& q, double t)
	{
		return { mix(p.x,q.x, t), mix(p.y, q.y, t) };
	};

	double dist(const WidePoint& a, const WidePoint& b)
	{
		auto dx = a.x - b.x;
		auto dy = a.y - b.y;
		auto result = dx * dx + dy * dy;
		return result;
	}
	
	struct AngleComparer
	{
		std::vector<WidePoint>& v;
		WidePoint c;
		AngleComparer(std::vector<WidePoint>& verts, const WidePoint& center)
			: v(verts), c(center) {}
		bool operator() (int i, int j)
		{
			/*

			First, find the center of the bounding box that contains all of your vertices. We'll call this point C.

			Sort your list of vertices based on each point's angle with respect to C.
			You can use atan2(point.y - C.y, point.x - C.x) to find the angle.
			If two or more vertices have the same angle, the one closer to C should come first.

			*/
			auto ai = atan2(v[i].y - c.y, v[i].x - c.x);
			auto aj = atan2(v[j].y - c.y, v[j].x - c.x);
			if (ai == aj)
			{
				return dist(v[i], c) > dist(v[j], c);
			}
			return ai > aj;
		}
	};


	struct DistanceComparer
	{
		WidePoint last;
		DistanceComparer(const WidePoint& seed) : last(seed) {}
		bool operator() (const WidePoint& a, const WidePoint& b)
		{
			bool ret = dist(a, last) < dist(b, last);
			last = ret ? a : b;
			return ret;
		}
	};
	
}

//s_begin_r(s) = _s_start_r[s] = triangles
//s_end_r(s) = _s_start_r[s_next_s(s)] = triangles[Next(s)]
//s_inner_t(s) = s_to_t(s)
//s_outer_t(s) = s_to_t(_s_opposite_s[s]) = s_to_t(half_edges[s])



LandGenerator::LandGenerator(uint32_t seed)
	: _seed(seed)
	, _max_bounds{ 25.0, 25.0 }
	, _mesh(seed, _max_bounds, 0.15)
	, _water_regions(_mesh.GetRegionCount(), true)
	, _coastal_regions(_mesh.GetRegionCount(), false)
	, _ocean_regions(_mesh.GetRegionCount(), false)
	, _perlin(seed)
	, _randomizer(seed)
{
	//_CreateNoisyEdges();
	_AssignWaterRegions();
	_AssignOceanRegions();
	_AssignCoastalRegions();
	_coastline_vertices = GetCoastlines();
}


void LandGenerator::_RecursiveSubdivide(std::vector<WidePoint>& points, double length, double amplitude, const WidePoint& a, const WidePoint& b, const WidePoint& p, const WidePoint& q)
{
	const int divisor = 0x10000000;
	double dx = a.x - b.x;
	double dy = a.y - b.y;
	if (dx*dx + dy*dy < length*length)
	{
		points.push_back(b);
		return;
	}

	auto ap = mixp(a, p, 0.5);
	auto bp = mixp(b, p, 0.5);
	auto aq = mixp(a, q, 0.5);
	auto bq = mixp(b, q, 0.5);

	auto division = 0.5 * (1 - amplitude) + _randomizer(divisor) / divisor * amplitude;
	auto center = mixp(p, q, division);

	_RecursiveSubdivide(points, length, amplitude, a, center, ap, aq);
	_RecursiveSubdivide(points, length, amplitude, center, b, bp, bq);
};


/*
a region is ocean if it is a water region connected to the ghost region,
which is outside the boundary of the map; this could be any seed set but
for islands, the ghost region is a good seed  */

void LandGenerator::_AssignOceanRegions()
{
	std::stack<int> unchecked_regions;
	unchecked_regions.push(_mesh.GetGhostIndexVerts());

	while (!unchecked_regions.empty()) 
	{
		auto r1 = unchecked_regions.top();
		unchecked_regions.pop();
		auto neighbors = _mesh.GetRegionNeighbors(r1);
		for (auto& neighbor: neighbors) 
		{
			if (IsWater(neighbor) && !IsOcean(neighbor))
			{
				_ocean_regions[neighbor] = true;
				unchecked_regions.push(neighbor);
			}
		}
	}
}




std::vector<int> LandGenerator::GetCoastlines()
{
	std::vector<int> coasts;
	for (int s = 0; s < _mesh.triangles.size(); s++)
	{
		auto r0 = _mesh.triangles[s];
		auto r1 = _mesh.triangles[Next(s)];
		auto t0 = s_to_t(_mesh.half_edges[s]);
		if (IsOcean(r0) && !IsOcean(r1))
		{
			// It might seem that we also need to check !r_ocean[r0] && r_ocean[r1]
			// and it might seem that we have to add both t and its opposite but
			// each t vertex shows up in *four* directed sides, so we only have to test
			// one fourth of those conditions to get the vertex in the list once.
			coasts.push_back(t0);
		}
	}

	return coasts;
}

double LandGenerator::_Noise(double x, double y, const std::vector<double>& amplitudes)
{
	//shape: {round: 0.5, inflate: 0.4, amplitudes: [1/2, 1/4, 1/8, 1/16]},
	double sum = 0.0;
	double sumOfAmplitudes = 0.0;
	for (size_t octave = 0; octave < amplitudes.size(); octave++) 
	{
		auto frequency = 1 << octave;
		sum += amplitudes[octave] * _perlin.octaveNoise(x * frequency, y * frequency, octave);
		sumOfAmplitudes += amplitudes[octave];
	}
	return sum / sumOfAmplitudes;
}

std::vector<int> LandGenerator::_FollowCoastline(int t0, std::set<int>& visited)
{
	std::vector<int> output;
	int t = t0;
	int last = t;
	do
	{
		output.push_back(t);
		auto neighbors = _mesh.GetRegionVertexNeighbors(t);
		for (auto& neighbor : neighbors)
		{
			// If neighbor vertex borders ocean and this is the first visit or we reached home
			if (IsCoastlineVertex(neighbor) && IsCoastEdge(t, neighbor) && (visited.insert(neighbor).second || (neighbor == t0 && t0 != last)))
			{
				last = t;
				t = neighbor;
				break;
			}
		}

	} while (t != t0);

	return output;
}

bool LandGenerator::IsCoastEdge(int t1, int t2)
{
	int r1, r2;
	_mesh.GetFlankingRegions(t1, t2, r1, r2);
	if (r1 == -1) return false;
	return (IsOcean(r1) && !IsOcean(r2)) || (IsOcean(r2) && !IsOcean(r1));
}

std::vector<std::vector<int>> LandGenerator::GetCoastlines2()
{
	std::vector<std::vector<int>> output;
	std::set<int> visited;
	
	for (int j = 0; j < _coastline_vertices.size(); ++j)
	{
		// if it has not been visited
		if (visited.insert(_coastline_vertices[j]).second)
		{
			// Follow the coastline back to the start
			std::vector<int> coastline = _FollowCoastline(_coastline_vertices[j], visited);
			output.push_back(coastline);
		}
	}

	return output;

}

void LandGenerator::_AssignCoastalRegions()
{
	for (int i = 0; i < _mesh.GetRegionCount(); ++i) 
	{
		auto neighbors = _mesh.GetRegionNeighbors(i);
		if (!IsWater(i)) 
		{
			for(auto& neighbor: neighbors)
			{
				if (IsWater(neighbor))
				{
					_coastal_regions[i] = true;
					break;
				}
			}
		}
	}
}

std::vector<WidePoint> LandGenerator::GetNoisyEdges(int edge_index)
{
	return _noisy_edges[edge_index];
}

void LandGenerator::_CreateNoisyEdges()
{
	auto ghost_index_tris = _mesh.GetGhostIndexTris();
	Randomizer random_int(_seed);
	double length = 4.0;
	double amplitude = 0.2;
	// numSides = triangles.size()
	for (int s = 0; s < _mesh.triangles.size(); ++s)
	{
		int t0 = s_to_t(s);
		int t1 = s_to_t(_mesh.half_edges[s]);
		int r0 =  _mesh.triangles[s];
		int r1 = _mesh.triangles[Next(s)];

		if (r0 < r1) 
		{
			if (s >= ghost_index_tris)
			{
				_noisy_edges[s].push_back(_mesh.region_vertices[t1]);
			}
			else {
				
				_RecursiveSubdivide(_noisy_edges[s], length, amplitude,
					_mesh.region_vertices[t0],
					_mesh.region_vertices[t1],
					_mesh.vertices[r0],
					_mesh.vertices[r1]
					);
			}
			// construct line going the other way; since the line is a
			// half-open interval with [p1, p2, p3, ..., pn] but not
			// p0, we want to reverse all but the last element, and
			// then append p0

			_noisy_edges[_mesh.half_edges[s]] = std::vector<WidePoint>(_noisy_edges[s].rbegin()+1, _noisy_edges[s].rend());
			_noisy_edges[_mesh.half_edges[s]].push_back(_mesh.region_vertices[t0]);
		}
	}
}

void LandGenerator::_AssignWaterRegions()
{
	int ghost_index = _mesh.GetGhostIndexVerts();
	auto center = _mesh.Center();
	// 1/2, 1/4, 1/8, 1/16
	std::vector<double> amplitudes { 0.5, 0.25, 0.125, 0.0625 };
	double roundness = 0.25;//0.5;
	double inflation = 0.4;

	for (int r = 0; r < ghost_index; r++)
	{
		if (!_mesh.IsBoundaryRegion(r)) 
		{
			auto nx = (_mesh.vertices[r].x - center.x) / center.x;
			auto ny = (_mesh.vertices[r].y - center.y) / center.y;
			auto distance = max(abs(nx), abs(ny));
			auto n = _Noise(nx, ny, amplitudes);
			n = mix(n, 0.5, roundness);
			_water_regions[r] = n - (1.0 - inflation) * distance * distance < 0;
		}
	}
}

