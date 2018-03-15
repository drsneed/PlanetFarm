#pragma once
#include <Core/GraphicsWindow.h>
#include "CameraBehavior.h"

class Camera
{
public:
	XMVECTOR GetLookVectorXM() const { return XMLoadFloat3(&m_look); }
	XMVECTOR GetUpVectorXM() const { return XMLoadFloat3(&m_up); }
	XMVECTOR GetRightVectorXM() const { return XMLoadFloat3(&m_right); }
	void Yaw(float amount) { m_yaw += XMConvertToRadians(amount); }
	void Pitch(float amount) { m_pitch += XMConvertToRadians(amount); }
	void Roll(float amount) { m_roll += XMConvertToRadians(amount); }

	void UpdateGpuBuffer()
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

	size_t NotifyPosChange(std::function<void(XMFLOAT3)> callback)
	{
		_notify_pos_change_list.push_back(callback);
		return _notify_pos_change_list.size() - 1;
	}
	
protected:
	ID3D11Buffer* m_gpuCameraBuffer;
	XMFLOAT4X4 m_viewMatrix;
	XMFLOAT4X4 m_projMatrix;
	XMFLOAT4X4 m_viewportMatrix;
	XMFLOAT3 m_position;
	XMFLOAT3 m_right;
	XMFLOAT3 m_up;
	XMFLOAT3 m_look;
	float m_yaw;
	float m_pitch;
	float m_roll;

	std::vector <std::function<void(XMFLOAT3)>> _notify_pos_change_list;

	std::shared_ptr<CameraBehavior> m_behavior;

	struct ConstantBuffer
	{
		XMMATRIX ViewMatrix;
		XMMATRIX ProjectionMatrix;
		XMMATRIX ViewProjectionMatrix;
		XMMATRIX ViewportMatrix;
		XMFLOAT3 CameraPosition;
		float padding;
	};


	

public:

	Camera(std::shared_ptr<CameraBehavior> behavior)
		: m_gpuCameraBuffer(nullptr)
		, m_position(6.0f, 20.0f, 6.0f)
		, m_right(1.0f, 0.0f, 0.0f)
		, m_up(0.0f, 1.0f, 0.0f)
		, m_look(0.0f, 0.0f, 1.0f)
		, m_yaw(0)
		, m_pitch(0)
		, m_roll(0)
		, m_behavior(behavior)

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

	~Camera()
	{
		if (m_gpuCameraBuffer)
			m_gpuCameraBuffer->Release();
	}

	void Tick(float dt) 
	{ 
		m_behavior->Tick(this, dt);
		UpdateGpuBuffer();
	}

	void HandleEvent(GraphicsWindow::Event& event) 
	{ 
		m_behavior->HandleEvent(this, event); 
	};

	void ComputeRayFromMouseCursor(XMFLOAT3& origin, XMFLOAT3& direction)
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

	void SetLens(float fovY, float aspect, float zn, float zf)
	{
		XMMATRIX P = XMMatrixPerspectiveFovLH(fovY, aspect, zn, zf);
		XMStoreFloat4x4(&m_projMatrix, P);
	}

	void OnResize(int width, int height)
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
	XMMATRIX GetViewMatrix() const { return XMLoadFloat4x4(&m_viewMatrix); }
	XMMATRIX GetProjectionMatrix() const { return XMLoadFloat4x4(&m_projMatrix); }
	XMMATRIX GetViewportMatrix() const { return XMLoadFloat4x4(&m_viewportMatrix); }
	XMMATRIX GetViewProjectionMatrix() const { return GetViewMatrix() * GetProjectionMatrix(); }
	ID3D11Buffer* GetConstantBuffer() const { return m_gpuCameraBuffer; }
	XMFLOAT3 GetPosition() const { return m_position; };
	XMVECTOR GetPositionXM() const { return XMLoadFloat3(&m_position); };

	void SetPosition(float x, float y, float z)
	{
		m_position.x = x;
		m_position.y = y;
		m_position.z = z;
		for(auto& callback: _notify_pos_change_list)
		{
			callback(m_position);
		}
	}
	void SetPosition(const XMFLOAT3& position)
	{
		m_position = position;
		for (auto& callback : _notify_pos_change_list)
		{
			callback(m_position);
		}
	}

};