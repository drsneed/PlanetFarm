#include "StdIncludes.h"
#include "Shader.h"
#include "Logger.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler")




HRESULT CompileShader(const WCHAR* filename, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** ppBlob)
{
	auto logger = Logger::GetInstance();

	DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* errorBlob = nullptr;
	auto hr = D3DCompileFromFile(filename, nullptr, nullptr, entryPoint, shaderModel, flags, 0, ppBlob, &errorBlob);
	if (hr != S_OK)
	{
		if (errorBlob)
		{
			logger->WriteError(L"Failed to compile shader '%s'. Reason: %S\n", filename,
				reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}
		else
		{
			logger->WriteError(L"Failed to compile shader '%s'. HR: 0x%x, %s\n", filename,
				hr, GetHRESULTErrorMessage(hr));
		}
		SetD3DErrorOccurred();
		return hr;
	}

	if (errorBlob)
	{
		errorBlob->Release();
	}

	return S_OK;
}


Shader::Shader(const WCHAR* filename, int8_t types, const char* vsEP, const char* psEP)
	: m_types(types)
	, m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
	, m_hullShader(nullptr)
	, m_domainShader(nullptr)
	, m_computeShader(nullptr)
	, m_geometryShader(nullptr)
	, m_vertexShaderBC(nullptr)
	, m_pixelShaderBC(nullptr)
	, m_hullShaderBC(nullptr)
	, m_domainShaderBC(nullptr)
	, m_computeShaderBC(nullptr)
	, m_geometryShaderBC(nullptr)
{
	auto graphicsWindow = GraphicsWindow::GetInstance();
	auto device = graphicsWindow->GetDevice();

	if (types & Vertex)
	{
		if (FAILED(CompileShader(filename, vsEP, "vs_5_0", &m_vertexShaderBC)))
		{
			return;
		}

		if(!D3DCheck(device->CreateVertexShader(
			m_vertexShaderBC->GetBufferPointer(),
			m_vertexShaderBC->GetBufferSize(),
			nullptr,
			&m_vertexShader), L"ID3D11Device::CreateVertexShader")) return;
	}

	if (types & Hull)
	{
		if (FAILED(CompileShader(filename, "HullShader", "hs_5_0", &m_hullShaderBC)))
		{
			return;
		}

		if(!D3DCheck(device->CreateHullShader(
			m_hullShaderBC->GetBufferPointer(),
			m_hullShaderBC->GetBufferSize(),
			nullptr,
			&m_hullShader), L"ID3D11Device::CreateVertexShader")) return;
	}

	if (types & Domain)
	{
		if (FAILED(CompileShader(filename, "DomainShader", "ds_5_0", &m_domainShaderBC)))
		{
			return;
		}

		if (!D3DCheck(device->CreateDomainShader(
			m_domainShaderBC->GetBufferPointer(),
			m_domainShaderBC->GetBufferSize(),
			nullptr,
			&m_domainShader), L"ID3D11Device::CreateVertexShader")) return;
	}

	if (types & Compute)
	{
		if (FAILED(CompileShader(filename, "ComputeShader", "cs_5_0", &m_computeShaderBC)))
		{
			return;
		}

		if (!D3DCheck(device->CreateComputeShader(
			m_computeShaderBC->GetBufferPointer(),
			m_computeShaderBC->GetBufferSize(),
			nullptr,
			&m_computeShader), L"ID3D11Device::CreateVertexShader")) return;
	}

	if (types & Geometry)
	{
		if (FAILED(CompileShader(filename, "GS", "gs_5_0", &m_geometryShaderBC)))
		{
			return;
		}

		if (!D3DCheck(device->CreateGeometryShader(
			m_geometryShaderBC->GetBufferPointer(),
			m_geometryShaderBC->GetBufferSize(),
			nullptr,
			&m_geometryShader), L"ID3D11Device::CreateVertexShader")) return;
	}

	if (types & Pixel)
	{
		if (FAILED(CompileShader(filename, psEP, "ps_5_0", &m_pixelShaderBC)))
		{
			return;
		}
		if (!D3DCheck(device->CreatePixelShader(
			m_pixelShaderBC->GetBufferPointer(),
			m_pixelShaderBC->GetBufferSize(),
			nullptr,
			&m_pixelShader), L"ID3D11Device::CreateVertexShader")) return;
	}
}

Shader::~Shader()
{
	ReleaseByteCode();
	if (m_vertexShader)
		m_vertexShader->Release();
	if (m_pixelShader)
		m_pixelShader->Release();
	if (m_hullShader)
		m_hullShader->Release();
	if (m_domainShader)
		m_domainShader->Release();
	if (m_computeShader)
		m_computeShader->Release();
	if (m_geometryShader)
		m_geometryShader->Release();
}

Shader::operator bool()
{
	if (m_types & Vertex && !m_vertexShader)
		return false;
	if (m_types & Pixel && !m_pixelShader)
		return false;
	if (m_types & Geometry && !m_geometryShader)
		return false;
	if (m_types & Compute && !m_computeShader)
		return false;
	if (m_types & Domain && !m_domainShader)
		return false;
	if (m_types & Hull && !m_hullShader)
		return false;
	return true;
}

void Shader::ReleaseByteCode()
{
	if(m_vertexShaderBC)
	{
		m_vertexShaderBC->Release();
		m_vertexShaderBC = nullptr;
	}

	if (m_pixelShaderBC)
	{
		m_pixelShaderBC->Release();
		m_pixelShaderBC = nullptr;
	}
	if (m_hullShaderBC)
	{
		m_hullShaderBC->Release();
		m_hullShaderBC = nullptr;
	}
	if (m_domainShaderBC)
	{
		m_domainShaderBC->Release();
		m_domainShaderBC = nullptr;
	}
	if (m_computeShaderBC)
	{
		m_computeShaderBC->Release();
		m_computeShaderBC = nullptr;
	}
	if (m_geometryShaderBC)
	{
		m_geometryShaderBC->Release();
		m_geometryShaderBC = nullptr;
	}
}

ID3DBlob* Shader::GetByteCode(Type type)
{
	switch (type)
	{
	case Vertex:
		return m_vertexShaderBC;
	case Pixel:
		return m_pixelShaderBC;
	case Hull:
		return m_hullShaderBC;
	case Domain:
		return m_domainShaderBC;
	case Compute:
		return m_computeShaderBC;
	case Geometry:
		return m_geometryShaderBC;
	default:
		return nullptr;
	}
}

/*void Shader::BindAll()
{
	auto context = gpu->GetDeviceContext();
	if (m_types & Vertex)
	{
		context->VSSetShader(m_vertexShader, nullptr, 0);
	}
	if (m_types & Geometry)
	{
		context->GSSetShader(m_geometryShader, nullptr, 0);
	}
	if (m_types & Pixel)
	{
		context->PSSetShader(m_pixelShader, nullptr, 0);
	}
}*/