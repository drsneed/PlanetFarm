#pragma once
#include <Core/StdIncludes.h>
#include <Core/Texture.h>
#include <Core/Shader.h>
#include "Camera.h"
#include "Models/Grid.h"
#include "Models/Square.h"

class MapRenderer
{
	Shader m_gridShader;
	ID3D11InputLayout* m_gridInputLayout;
	WorldGrid m_grid;

	Shader m_squareShader;
	ID3D11InputLayout* m_squareInputLayout;
	Square m_square;

	__declspec(align(16)) struct SquarePerObjectBuffer
	{
		XMFLOAT4 ObjectColor;
		XMFLOAT2 ObjectPosition;
		XMFLOAT2 ObjectSize;
	};

	ID3D11Buffer* m_perObjectBuffer;

	void _UploadPerObjectBuffer(ID3D11DeviceContext* context, const SquarePerObjectBuffer& perObject);

public:
	MapRenderer();
	~MapRenderer();

	void DrawGrid(std::shared_ptr<Camera>& camera);
	void DrawSquare(std::shared_ptr<Camera>& camera, XMFLOAT2 position, float width, unsigned color);
};
