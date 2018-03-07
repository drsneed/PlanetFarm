#include "Camera.h"

namespace
{
	bool frozen = false;
	XMFLOAT3 oldPosition;
}

#define INITIAL_SPEED 20.f

Camera::Camera()
	: ICamera()
	, m_speed(INITIAL_SPEED)
	, m_rotationEnabled(false)
	, m_ctrlDown(false)
{

}

Camera::~Camera()
{
}
void Camera::_HandleMovement(float dt)
{
	auto window = GraphicsWindow::GetInstance();
	if (window->IsKeyDown('W'))
	{
		Walk(m_speed * dt);
	}
	if (window->IsKeyDown('S'))
	{
		Walk(-m_speed * dt);
	}
	if (window->IsKeyDown('A'))
	{
		Strafe(-m_speed * dt);
	}
	if (window->IsKeyDown('D'))
	{
		Strafe(m_speed * dt);
	}
	if (window->IsKeyDown('Q'))
	{
		Zoom(m_speed * dt);
	}
	if (window->IsKeyDown('E'))
	{
		Zoom(-m_speed * dt);
	}
}

void Camera::_HandleMatrixUpdate()
{
	if (m_changes.Any())
	{
		XMVECTOR R = XMLoadFloat3(&m_right);
		XMVECTOR L = XMLoadFloat3(&m_look);
		XMVECTOR P = XMLoadFloat3(&m_position);

		// Normalize look
		L = XMVector3Normalize(L);
		// Compute a new corrected "Up" vector and normalize it
		XMVECTOR U = XMVector3Normalize(XMVector3Cross(L, R));

		// Compute a new corrected "right" vector. U and L are already
		// ortho-normal, so no need to normalize cross product
		// ||up x look|| = ||up|| * ||look|| * sin90 = 1
		R = XMVector3Cross(U, L);

		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, L));

		m_viewMatrix(0, 0) = m_right.x;
		m_viewMatrix(1, 0) = m_right.y;
		m_viewMatrix(2, 0) = m_right.z;
		m_viewMatrix(3, 0) = x;

		m_viewMatrix(0, 1) = m_up.x;
		m_viewMatrix(1, 1) = m_up.y;
		m_viewMatrix(2, 1) = m_up.z;
		m_viewMatrix(3, 1) = y;

		m_viewMatrix(0, 2) = m_look.x;
		m_viewMatrix(1, 2) = m_look.y;
		m_viewMatrix(2, 2) = m_look.z;
		m_viewMatrix(3, 2) = z;

		m_viewMatrix(0, 3) = 0.0f;
		m_viewMatrix(1, 3) = 0.0f;
		m_viewMatrix(2, 3) = 0.0f;
		m_viewMatrix(3, 3) = 1.0f;

		__declspec(align(16)) ConstantBuffer cameraBuffer;
		cameraBuffer.ViewMatrix = XMMatrixTranspose(GetViewMatrix());
		cameraBuffer.ViewProjectionMatrix = XMMatrixTranspose(GetViewProjectionMatrix());
		cameraBuffer.ProjectionMatrix = XMMatrixTranspose(GetProjectionMatrix());
		cameraBuffer.ViewportMatrix = XMMatrixTranspose(GetViewportMatrix());
		//cameraBuffer.ViewportMatrix = GetViewportMatrix();
		if (frozen)
		{
			cameraBuffer.CameraPosition = oldPosition;
		}
		else
		{
			cameraBuffer.CameraPosition = m_position;
			oldPosition = m_position;

		}


		D3D11_MAPPED_SUBRESOURCE mappedRes;
		auto context = GraphicsWindow::GetInstance()->GetContext();
		if(!D3DCheck(context->Map(m_gpuCameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes), 
			L"ID3D11DeviceContext::Map (Camera Matrix Update)")) return;
		memcpy_s(mappedRes.pData, sizeof(ConstantBuffer), &cameraBuffer, sizeof(ConstantBuffer));
		context->Unmap(m_gpuCameraBuffer, 0);
	}
}


void Camera::Update(float dt, bool gameUIHasFocus)
{
	auto window = GraphicsWindow::GetInstance();
	m_ctrlDown = window->IsKeyDown(VK_LCONTROL);
	if (!gameUIHasFocus)
	{
		_HandleMovement(dt);
	}
	_HandleMatrixUpdate();
}

void Camera::OnMouseMove(int x, int y)
{
	if (m_rotationEnabled)
	{
		auto dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_lastMouseX));
		auto dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_lastMouseY));
		Pitch(dy);
		RotateY(dx);
		m_lastMouseX = x;
		m_lastMouseY = y;
	}
}

void Camera::EnableRotateView(bool enable, int mouseX, int mouseY)
{
	m_rotationEnabled = enable;

	if (enable)
	{
		m_lastMouseX = mouseX;
		m_lastMouseY = mouseY;
	}

}




auto Camera::GetCurrentSpeedMultiplier() const -> float
{
	return m_speed / INITIAL_SPEED;
}

XMVECTOR Camera::GetRightXM() const
{
	return XMLoadFloat3(&m_right);
}

XMFLOAT3 Camera::GetRight() const
{
	return m_right;
}

XMVECTOR Camera::GetUpXM() const
{
	return XMLoadFloat3(&m_up);
}

XMFLOAT3 Camera::GetUp() const
{
	return m_up;
}

XMVECTOR Camera::GetLookXM() const
{
	return XMLoadFloat3(&m_look);
}

float Camera::GetNearZ() const
{
	return m_nearZ;
}

float Camera::GetFarZ() const
{
	return m_farZ;
}

float Camera::GetAspect() const
{
	return m_aspect;
}

float Camera::GetFOVY() const
{
	return m_fovY;
}

float Camera::GetFOVX() const
{
	float halfWidth = 0.5f * GetNearWindowWidth();
	return 2.0f * atan(halfWidth / m_nearZ);
}

float Camera::GetNearWindowWidth() const
{
	return m_aspect * m_nearWindowHeight;
}

float Camera::GetNearWindowHeight() const
{
	return m_nearWindowHeight;
}

float Camera::GetFarWindowWidth() const
{
	return m_aspect * m_farWindowHeight;
}

float Camera::GetFarWindowHeight() const
{
	return m_farWindowHeight;
}

void Camera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&m_position, pos);
	XMStoreFloat3(&m_look, L);
	XMStoreFloat3(&m_right, R);
	XMStoreFloat3(&m_up, U);
}

void Camera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);
}

void Camera::Strafe(float d)
{
	// m_position += d * m_right
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&m_right);
	XMVECTOR p = XMLoadFloat3(&m_position);
	XMStoreFloat3(&m_position, XMVectorMultiplyAdd(s, r, p));
	m_changes.Position = true;
}

void Camera::Walk(float d)
{
	// m_position += d * m_look;
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&m_right);
	XMVECTOR p = XMLoadFloat3(&m_position);
	auto up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	XMVECTOR c = XMVector3Cross(r, up);
	XMStoreFloat3(&m_position, XMVectorMultiplyAdd(s, c, p));
	m_changes.Position = true;
	/*	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&m_look);
	XMVECTOR p = XMLoadFloat3(&m_position);
	XMStoreFloat3(&m_position, XMVectorMultiplyAdd(s, l, p));
	m_changes.Position = true;*/
}

void Camera::Pitch(float angle)
{
	// Rotate up and look vector about the right vector.

	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&m_right), angle);

	XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), R));
	XMStoreFloat3(&m_look, XMVector3TransformNormal(XMLoadFloat3(&m_look), R));

	m_changes.Rotation = true;
}

void Camera::RotateY(float angle)
{
	// Rotate the basis vectors about the world y-axis.

	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&m_right, XMVector3TransformNormal(XMLoadFloat3(&m_right), R));
	XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), R));
	XMStoreFloat3(&m_look, XMVector3TransformNormal(XMLoadFloat3(&m_look), R));

	m_changes.Rotation = true;
}

void Camera::Zoom(float d)
{
	m_position.y += d;
	m_changes.Position = true;
}

void Camera::Freeze()
{
	frozen = true;
}

void Camera::HandleEvent(GraphicsWindow::Event& event)
{
	switch (event.type)
	{
	case GraphicsWindow::Event::Type::MouseClick:
		if (event.code == GraphicsWindow::Event::Code::MouseMiddle)
			EnableRotateView(true, event.x, event.y);
		break;

	case GraphicsWindow::Event::Type::MouseRelease:
		if (event.code == GraphicsWindow::Event::Code::MouseMiddle)
			EnableRotateView(false, 0, 0);
		break;

	case GraphicsWindow::Event::Type::WindowResize:
		OnResize(event.x, event.y);
		break;

	case GraphicsWindow::Event::Type::MouseMotion:
		OnMouseMove(event.x, event.y);
		break;

	case GraphicsWindow::Event::Type::MouseWheelUp:
		m_speed *= 2.0f;
		break;

	case GraphicsWindow::Event::Type::MouseWheelDown:
		m_speed *= 0.5f;
		break;

	default: break;
	}
}