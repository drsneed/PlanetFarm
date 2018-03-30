#include "LandGenerator.h"
#include "DiskSampler.h"
#include "Triangulator.h"

LandGenerator::LandGenerator()
{

}

TempLand LandGenerator::GetLand()
{
	TempLand temp;
	temp.vertices = DiskSampler::GetSamples();
	Triangulator triangulator(temp.vertices);
	temp.tris = triangulator.Triangles();
	return temp;
}
