#pragma once
#include <Core/StdIncludes.h>
#include <Core/Texture.h>
#include <Core/Shader.h>
#include "Camera.h"
#include "Models/Grid.h"

class WorldRenderer
{
	Shader m_gridShader;
	ID3D11InputLayout* m_gridInputLayout;
	WorldGrid m_grid;

public:
	WorldRenderer();
	~WorldRenderer();

	//void RenderVoxel(const VoxelModel::VoxelInstance& cube, std::shared_ptr<Texture>& texture, std::shared_ptr<Camera> camera);
	void RenderGrid(std::shared_ptr<Camera>& camera);
};
