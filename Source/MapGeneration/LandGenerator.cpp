#include "LandGenerator.h"
#include "Triangulator.h"
#include <Core/DebugTools.h>

namespace
{
	inline double mix(double a, double b, double t) 
	{
		return a * (1.0 - t) + b * t;
	};
}

LandGenerator::LandGenerator(uint32_t seed)
	: _mesh(seed, { 20.0, 20.0 }, 1.0)
	, _water_regions(_mesh.GetRegionCount(), true)
	, _coastal_regions(_mesh.GetRegionCount(), false)
	, _perlin(seed)
{
	_CreateNoisyEdges();
	_AssignWaterRegions();
	_AssignCoastalRegions();
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

void LandGenerator::_CreateNoisyEdges()
{
	auto ghost_index_tris = _mesh.GetGhostIndexTris();
	// numSides = triangles.size()
	for (int s = 0; s < _mesh.triangles.size(); ++s)
	{
		int t0 = 0;//mesh.s_inner_t(s);
		int t1 = 0;//mesh.s_outer_t(s);
		int r0 = 0;// mesh.s_begin_r(s);
		int r1 = 0;// mesh.s_end_r(s);

		if (r0 < r1) 
		{
			if (s >= ghost_index_tris)
			{
				s_lines[s] = [mesh.t_pos([], t1)];
			}
			else {
				s_lines[s] = exports.recursiveSubdivision(length, amplitude, randInt)(
					mesh.t_pos([], t0),
					mesh.t_pos([], t1),
					mesh.r_pos([], r0),
					mesh.r_pos([], r1)
					);
			}
			// construct line going the other way; since the line is a
			// half-open interval with [p1, p2, p3, ..., pn] but not
			// p0, we want to reverse all but the last element, and
			// then append p0
			let opposite = s_lines[s].slice(0, -1);
			opposite.reverse();
			opposite.push(mesh.t_pos([], t0));
			s_lines[mesh.s_opposite_s(s)] = opposite;
		}
	}
	return s_lines;
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

