#include "ICamera.h"

ICamera::ICamera()
	: m_gpuCameraBuffer(nullptr)
	, m_position(6.0f, 20.0f, 6.0f)
	, m_right(1.0f, 0.0f, 0.0f)
	, m_up(0.0f, 1.0f, 0.0f)
	, m_look(0.0f, 0.0f, 1.0f)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	auto window = GraphicsWindow::GetInstance();
	if (!D3DCheck(window->GetDevice()->CreateBuffer(&bd, nullptr, &m_gpuCameraBuffer),
		L"ID3D11Device::CreateBuffer (Camera)")) return;

	int width, height;
	window->GetSize(width, height);
	OnResize(width, height);
}

ICamera::~ICamera()
{
	if (m_gpuCameraBuffer)
		m_gpuCameraBuffer->Release();
}

void ICamera::ComputeRayFromMousePointer(XMFLOAT3& origin, XMFLOAT3& direction)
{
	auto window = GraphicsWindow::GetInstance();
	int sx, sy, width, height;
	window->GetMousePosition(sx, sy);
	window->GetSize(width, height);

	auto vx = (2.0f * sx / width - 1.0f) / m_projMatrix(0, 0);
	auto vy = (-2.0f * sy / height + 1.0f) / m_projMatrix(1, 1);

	auto o = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	auto d = DirectX::XMVectorSet(vx, vy, 1.0f, 0.0f);

	auto view = GetViewMatrix();
	auto invView = DirectX::XMMatrixInverse(&XMMatrixDeterminant(view), view);

	// We need to take the world matrix into account when we start picking objects and not just terrain
	/*
	XMMATRIX W = XMLoadFloat4x4(&mMeshWorld);
	XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);
	XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

	*/
	o = DirectX::XMVector3TransformCoord(o, invView);
	d = DirectX::XMVector3TransformNormal(d, invView);
	d = XMVector3Normalize(d);
	XMStoreFloat3(&origin, o);
	XMStoreFloat3(&direction, d);
}

void ICamera::SetLens(float fovY, float aspect, float zn, float zf)
{
	XMMATRIX P = XMMatrixPerspectiveFovLH(fovY, aspect, zn, zf);
	XMStoreFloat4x4(&m_projMatrix, P);

	m_changes.Projection = true;
	
}

void ICamera::OnResize(int width, int height)
{
	SetLens(0.25f * XM_PI, width / static_cast<float>(height), 1.0f, 100000.0f);
	auto w2 = width / 2.0f;
	auto h2 = height / 2.0f;
	m_viewportMatrix = XMFLOAT4X4
		(
			w2, 0.f, 0.f, w2,
			0.f, h2, 0.f, h2,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
			);
}

XMMATRIX ICamera::GetViewMatrix() const
{
	return XMLoadFloat4x4(&m_viewMatrix);
}

XMMATRIX ICamera::GetProjectionMatrix() const
{
	return XMLoadFloat4x4(&m_projMatrix);
}

XMMATRIX ICamera::GetViewportMatrix() const
{
	return XMLoadFloat4x4(&m_viewportMatrix);
}

XMMATRIX ICamera::GetViewProjectionMatrix() const
{
	return GetProjectionMatrix() * GetViewMatrix();
}

ID3D11Buffer* ICamera::GetConstantBuffer() const
{
	return m_gpuCameraBuffer;
}

void ICamera::SetPosition(float x, float y, float z)
{
	m_position = XMFLOAT3(x, y, z);
	m_changes.Position = true;
}

void ICamera::SetPosition(const XMFLOAT3& position)
{
	m_position = position;
	m_changes.Position = true;
}
