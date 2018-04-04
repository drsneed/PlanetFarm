#include "LandGenerator.h"
#include "Triangulator.h"

LandGenerator::LandGenerator(uint32_t seed)
	: _mesh(seed, { 20.0, 20.0 }, 8.0)
{
	//std::vector<int>({ 1, 0, 3, 1, 2, 0 });
}

