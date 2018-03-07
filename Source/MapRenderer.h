#pragma once
#include "StdIncludes.h"
#include "Texture.h"
#include "Shader.h"
#include "Camera.h"
#include "Grid.h"

class WorldRenderer
{
	Shader m_gridShader;
	ID3D11InputLayout* m_gridInputLayout;
	WorldGrid m_grid;

public:
	WorldRenderer();
	~WorldRenderer();

	//void RenderVoxel(const VoxelModel::VoxelInstance& cube, std::shared_ptr<Texture>& texture, std::shared_ptr<Camera> camera);
	void RenderGrid(std::shared_ptr<ICamera> camera);
};
