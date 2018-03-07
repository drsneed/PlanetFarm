#pragma once
#include "GraphicsWindow.h"


HRESULT CompileShader(const WCHAR* filename, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** ppBlob);

class Shader
{
public:
	enum Type : int8_t
	{
		Vertex = 2,
		Hull = 4,
		Domain = 8,
		Compute = 16,
		Geometry = 32,
		Pixel = 64
	};

	explicit Shader(const WCHAR* filename, int8_t types, const char* vsEP = "VS", const char* psEP = "PS");
	~Shader();
	explicit operator bool();

	auto ReleaseByteCode() -> void;
	auto GetByteCode(Type type)->ID3DBlob*;
	auto GetVertexShader() -> ID3D11VertexShader* { return m_vertexShader; }
	auto GetPixelShader()  -> ID3D11PixelShader* { return m_pixelShader; }
	auto GetHullShader()   -> ID3D11HullShader* { return m_hullShader; }
	auto GetDomainShader() -> ID3D11DomainShader* { return m_domainShader; }
	auto GetComputeShader() -> ID3D11ComputeShader* { return m_computeShader; }
	auto GetGeometryShader() -> ID3D11GeometryShader* { return m_geometryShader; }

private:
	int8_t m_types;

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11HullShader* m_hullShader;
	ID3D11DomainShader* m_domainShader;
	ID3D11ComputeShader* m_computeShader;
	ID3D11GeometryShader* m_geometryShader;
	ID3DBlob* m_vertexShaderBC;
	ID3DBlob* m_pixelShaderBC;
	ID3DBlob* m_hullShaderBC;
	ID3DBlob* m_domainShaderBC;
	ID3DBlob* m_computeShaderBC;
	ID3DBlob* m_geometryShaderBC;
};