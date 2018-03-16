#pragma once
#include <Core/StdIncludes.h>
#include <Core/Texture.h>
#include <Core/Shader.h>
#include "Camera.h"
#include "Models/Grid.h"
#include "Models/Square.h"
#include <TileEngine/Tile.h>

class MapRenderer
{
	Shader m_gridShader;
	ID3D11InputLayout* m_gridInputLayout;
	WorldGrid m_grid;

	Shader m_squareShader;
	ID3D11InputLayout* m_squareInputLayout;
	Square m_square;

	Shader _map_bounds_shader;
	ID3D11InputLayout* _map_bounds_input_layout;
	ID3D11Buffer* _map_bounds_buffer;

	__declspec(align(16)) struct SquarePerObjectBuffer
	{
		XMFLOAT4X4 world_matrix;
		XMFLOAT4 color;
	};

	ID3D11Buffer* m_perObjectBuffer;

	void _UploadPerObjectBuffer(ID3D11DeviceContext* context, const SquarePerObjectBuffer& perObject);

	std::shared_ptr<Camera> _cam;

public:
	MapRenderer(std::shared_ptr<Camera> camera);
	~MapRenderer();

	void DrawTile(const Tile& tile);
	void DrawGrid();
	void DrawSquare(XMFLOAT2 position, float width, float rotation, unsigned color);
	void DrawMapBounds();
};
