#include "DiskSampler.h"
#include <cstdint>
#include <vector>
#include <thinks/poissondisksampling.hpp>
#include <Core/DebugTools.h>
using namespace std;



template <typename T, std::size_t N>
class Vec
{
public:
	typedef T value_type;
	static const std::size_t size = N;
	Vec() {}
	T& operator[](std::size_t i) { return _data[i]; }
	const T& operator[](std::size_t i) const { return _data[i]; }
private:
	T _data[N];
};

typedef Vec<double, 2> Vec2d;

std::vector<WidePoint> DiskSampler::GetSamples(uint32_t seed)
{
	// Setup input parameters.
	float radius = 2.0;
	WidePoint x_min;
	x_min[0] = -10.0;
	x_min[1] = -10.0;
	WidePoint x_max;
	x_max[0] = 10.0;
	x_max[1] = 10.0;
	uint32_t max_sample_attempts = 30;

	return thinks::poissonDiskSampling(radius, x_min, x_max, max_sample_attempts, seed);
}