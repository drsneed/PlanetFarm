#pragma once
#include <Core/StdIncludes.h>
#include "DualMesh.h"
#include "WidePoint.h"
#include <PerlinNoise.hpp>

class LandGenerator
{
	siv::PerlinNoise _perlin;
	DualMesh _mesh;
	std::vector<bool> _water_regions;
	std::vector<bool> _coastal_regions;
	std::map<int, std::vector<WidePoint>> _noisy_edges;
	void _AssignWaterRegions();
	void _AssignCoastalRegions();
	void _CreateNoisyEdges();
	double _Noise(double x, double y, const std::vector<double>& amplitudes);
public:
	LandGenerator(uint32_t seed);
	bool IsWater(int region_index) { return _water_regions[region_index]; }
	bool IsCoast(int region_index) { return _coastal_regions[region_index]; }
	DualMesh& GetMesh() { return _mesh; }
};
