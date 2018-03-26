#pragma once
#include <Game/Models/Cube.h>

class ModelsManager
{
	Cube _cube;
public:
	ModelsManager();
	Cube& GetCube();
};