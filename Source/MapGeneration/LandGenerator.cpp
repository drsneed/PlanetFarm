#include "LandGenerator.h"
#include "Triangulator.h"
#include <Core/DebugTools.h>

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
	

	
}

LandGenerator::LandGenerator(uint32_t seed)
	: _seed(seed)
	, _mesh(seed, { 20, 20 }, 1.0)
	, _water_regions(_mesh.GetRegionCount(), true)
	, _coastal_regions(_mesh.GetRegionCount(), false)
	, _perlin(seed)
	, _randomizer(seed)
{
	_CreateNoisyEdges();
	_AssignWaterRegions();
	_AssignCoastalRegions();
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

