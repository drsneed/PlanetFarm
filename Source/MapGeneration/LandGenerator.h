#pragma once
#include <Core/StdIncludes.h>
#include "DualMesh.h"
#include "WidePoint.h"
#include <PerlinNoise.hpp>
#include <map>

struct Randomizer
{
	std::default_random_engine randomizer;

	Randomizer(uint32_t seed)
		: randomizer(seed) {}

	int operator()(int n)
	{
		std::uniform_int_distribution<int> random_value(n);
		return random_value(randomizer);
	}
};

class LandGenerator
{
	uint32_t _seed;
	siv::PerlinNoise _perlin;
	DualMesh _mesh;
	Randomizer _randomizer;
	std::vector<bool> _water_regions;
	std::vector<bool> _coastal_regions;
	std::map<int, std::vector<WidePoint>> _noisy_edges;
	void _AssignWaterRegions();
	void _AssignCoastalRegions();
	void _CreateNoisyEdges();
	double _Noise(double x, double y, const std::vector<double>& amplitudes);
	void _RecursiveSubdivide(std::vector<WidePoint>& points, double length, double amplitude, 
		const WidePoint& a, const WidePoint& b, const WidePoint& p, const WidePoint& q);
public:
	LandGenerator(uint32_t seed);
	std::vector<WidePoint> GetNoisyEdges(int edge_index);
	bool IsWater(int region_index) { return _water_regions[region_index]; }
	bool IsCoast(int region_index) { return _coastal_regions[region_index]; }
	DualMesh& GetMesh() { return _mesh; }
};
