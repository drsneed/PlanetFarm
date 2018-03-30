#include "DiskSampler.h"
#include <cstdint>
#include <vector>
#include <thinks/poissondisksampling.hpp>

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

typedef Vec<float, 2> Vec2f;

std::vector<XMFLOAT2> DiskSampler::GetSamples()
{
	// Setup input parameters.
	float radius = 2.f;
	Vec2f x_min;
	x_min[0] = -10.f;
	x_min[1] = -10.f;
	Vec2f x_max;
	x_max[0] = 10.f;
	x_max[1] = 10.f;
	uint32_t max_sample_attempts = 30;
	uint32_t seed = 1981;

	vector<Vec2f> samples = thinks::poissonDiskSampling(radius, x_min, x_max, max_sample_attempts, seed);

	std::vector<XMFLOAT2> result(samples.size());
	for (int i = 0; i < samples.size(); ++i)
	{
		result[i] = XMFLOAT2(samples[i][0], samples[i][1]);
	}

	return result;
}