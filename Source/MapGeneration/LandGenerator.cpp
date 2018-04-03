#include "LandGenerator.h"
#include "DiskSampler.h"
#include "Triangulator.h"

LandGenerator::LandGenerator()
{

}

TempLand LandGenerator::GetLand()
{
	TempLand temp;
	temp.vertices = std::vector<XMFLOAT2> //DiskSampler::GetSamples();
	{ 
		{ -6.816375f, 4.808550f },
		{ 5.344729f, -2.469857f },
		{ -9.503366f, -7.519407f },
		{ 6.637861f, 7.446990f } 
	};
	temp.tris = std::vector<int>({ 1, 0, 3, 1, 2, 0 });
	std::vector<WidePoint> values
	{
		{ -6.816375, 4.808550 },
		{ 5.344729, -2.469857 },
		{ -9.503366, -7.519407 },
		{ 6.637861, 7.446990 }
	};
	Triangulator triangulator(values);
	return temp;
}
