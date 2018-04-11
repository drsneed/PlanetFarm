#pragma once
#include <Core/StdIncludes.h>
#include <Core/Texture.h>
#include <Core/Shader.h>
#include "Camera.h"
#include "Models/Grid.h"
#include "Models/Square.h"
#include <TileEngine/Tile.h>
#include <TileEngine/Models/Feature.h>
#include <TileEngine/ModelsManager.h>

class MapRenderer
{
	ModelsManager _models_manager;
	Shader m_gridShader;
	ID3D11InputLayout* m_gridInputLayout;
	WorldGrid m_grid;

	Shader m_squareShader;
	ID3D11InputLayout* m_squareInputLayout;
	Square m_square;

	Shader _map_bounds_shader;
	ID3D11InputLayout* _map_bounds_input_layout;
	ID3D11Buffer* _map_bounds_buffer;

	__declspec(align(16)) struct ModelPerObjectBuffer
	{
		XMFLOAT4X4 world_matrix;
		XMFLOAT4 color;
	};

	ID3D11Buffer* m_perObjectBuffer;

	void _UploadPerObjectBuffer(ID3D11DeviceContext* context, const ModelPerObjectBuffer& perObject);

	std::shared_ptr<Camera> _cam;

	Shader _static_feature_shader;
	ID3D11InputLayout* _static_feature_input_layout;

	ID3D11Buffer* _immediate_mode_buffer;

	

public:
	MapRenderer(std::shared_ptr<Camera> camera);
	~MapRenderer();
	void DrawTile(const Tile& tile, unsigned color);
	void DrawGrid();
	void DrawSquare(float x, float y, float width, float rotation, unsigned color);
	void DrawPoints(const std::vector<XMFLOAT2>& points, unsigned color);
	void DrawLine(const XMFLOAT2& from, const XMFLOAT2& to, unsigned color);
	void DrawLineStrip(const std::vector<XMFLOAT2>& verts, unsigned color);
	void DrawTriangle(const XMFLOAT2& a, const XMFLOAT2& b, const XMFLOAT2& c, unsigned color);
	void DrawMapBounds();
	void DrawStaticFeaturesBulk(StaticFeature* features_ptr, size_t features_count);
	void DrawDynamicFeaturesBulk(DynamicFeature* features_ptr, size_t features_count);
};
