#include "LandGenerator.h"
#include "DiskSampler.h"
#include "Triangulator.h"

LandGenerator::LandGenerator()
{

}

TempLand LandGenerator::GetLand(uint32_t seed)
{
	TempLand temp;
	temp.vertices = DiskSampler::GetSamples(seed);
	Triangulator triangulator(temp.vertices);
	temp.tris = triangulator.GetTriangles();//std::vector<int>({ 1, 0, 3, 1, 2, 0 });
	//auto half_edges = triangulator.GetHalfEdges();
	return temp;
}
