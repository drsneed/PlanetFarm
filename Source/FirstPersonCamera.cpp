#include "FirstPersonCamera.h"

FirstPersonCamera::FirstPersonCamera()
	: ICamera()
	, m_yaw(0) 
	, m_pitch(0)
	, m_roll(0)
	, m_lastMouseX(0)
	, m_lastMouseY(0)
	, m_up(0.0f, 1.0f, 0.0f)
	, m_look(0.0f, 0.0f, 1.0f)
	, m_right(1.0f, 0.0f, 0.0f)
{
	m_position = XMFLOAT3(0, 6, 0);
	_ResetWindowMousePos();
}

FirstPersonCamera::~FirstPersonCamera()
{
}

void FirstPersonCamera::Update(float dt, bool uiHasFocus)
{
	_HandleMatrixUpdate();
}

void FirstPersonCamera::_ResetWindowMousePos()
{
	auto window = GraphicsWindow::GetInstance();
	int width, height;
	window->GetSize(width, height);
	window->SetMousePosition(width / 2, height / 2);
	window->GetMousePosition(m_lastMouseX, m_lastMouseY);
}

void FirstPersonCamera::HandleEvent(GraphicsWindow::Event& event)
{
	switch (event.type)
	{
	case GraphicsWindow::Event::Type::MouseMotion:
	{
		auto dx = XMConvertToRadians(0.25f * static_cast<float>(event.x - m_lastMouseX));
		auto dy = XMConvertToRadians(0.25f * static_cast<float>(event.y - m_lastMouseY));
		Pitch(dy);
		Yaw(dx);
		_ResetWindowMousePos();
		break;
	}
	default: 
		break;
	}
}

void FirstPersonCamera::_HandleMatrixUpdate()
{
	if (m_changes.Any())
	{
		m_up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_look = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_right = XMFLOAT3(1.0f, 0.0f, 0.0f);

		XMVECTOR look = XMLoadFloat3(&m_look);
		XMVECTOR up = XMLoadFloat3(&m_up);
		XMVECTOR right = XMLoadFloat3(&m_right);

		auto pos = XMLoadFloat3(&m_position);

		auto yawMatrix = XMMatrixRotationAxis(up, m_yaw);
		look = XMVector3Transform(look, yawMatrix);
		right = XMVector3Transform(right, yawMatrix);

		auto pitchMatrix = XMMatrixRotationAxis(right, m_pitch);
		look = XMVector3Transform(look, pitchMatrix);
		up = XMVector3Transform(up, pitchMatrix);

/*		auto rollMatrix = XMMatrixRotationAxis(look, m_roll);
		right = XMVector3Transform(right, rollMatrix);
		up = XMVector3Transform(up, rollMatrix);*/

		FLOAT s_dot1, s_dot2, s_dot3;
		XMStoreFloat(&s_dot1, -XMVector3Dot(pos, right));
		XMStoreFloat(&s_dot2, -XMVector3Dot(pos, up));
		XMStoreFloat(&s_dot3, -XMVector3Dot(pos, look));

		XMStoreFloat3(&m_right, right);
		XMStoreFloat3(&m_look, look);
		XMStoreFloat3(&m_up, up);


		XMStoreFloat4x4(&m_viewMatrix, XMMatrixIdentity());

		m_viewMatrix._11 = m_right.x;
		m_viewMatrix._21 = m_right.y;
		m_viewMatrix._31 = m_right.z;

		m_viewMatrix._12 = m_up.x;
		m_viewMatrix._22 = m_up.y;
		m_viewMatrix._32 = m_up.z;

		m_viewMatrix._13 = m_look.x;
		m_viewMatrix._23 = m_look.y;
		m_viewMatrix._33 = m_look.z;

		m_viewMatrix._41 = s_dot1;
		m_viewMatrix._42 = s_dot2;
		m_viewMatrix._43 = s_dot3;

		__declspec(align(16)) ConstantBuffer cameraBuffer;
		cameraBuffer.ViewMatrix = XMMatrixTranspose(GetViewMatrix());
		cameraBuffer.ViewProjectionMatrix = XMMatrixTranspose(GetViewProjectionMatrix());
		cameraBuffer.ProjectionMatrix = XMMatrixTranspose(GetProjectionMatrix());
		cameraBuffer.ViewportMatrix = XMMatrixTranspose(GetViewportMatrix());

		D3D11_MAPPED_SUBRESOURCE mappedRes;
		auto context = GraphicsWindow::GetInstance()->GetContext();
		if (!D3DCheck(context->Map(m_gpuCameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes),
			L"ID3D11DeviceContext::Map (Camera Matrix Update)")) return;
		memcpy_s(mappedRes.pData, sizeof(ConstantBuffer), &cameraBuffer, sizeof(ConstantBuffer));
		context->Unmap(m_gpuCameraBuffer, 0);
	}
}

void FirstPersonCamera::Yaw(float amount)
{
	m_yaw += amount;
	m_changes.Rotation = true;
}

void FirstPersonCamera::Pitch(float amount)
{
	m_pitch += amount;
	m_changes.Rotation = true;
}

void FirstPersonCamera::Roll(float amount)
{
	m_roll += amount;
	m_changes.Rotation = true;
}
