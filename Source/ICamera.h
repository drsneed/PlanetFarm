#pragma once
#include "StdIncludes.h"
#include "GraphicsWindow.h"

class ICamera
{
public:


	struct Changes
	{
		bool Position : 1;
		bool Projection : 1;
		bool Rotation : 1;

		bool Any()
		{
			return Position || Projection || Rotation;
		}

		void Reset()
		{
			Position = Projection = Rotation = false;
		}
	};

	Changes& GetChanges() { return m_changes; };

protected:
	ID3D11Buffer* m_gpuCameraBuffer;
	XMFLOAT4X4 m_viewMatrix;
	XMFLOAT4X4 m_projMatrix;
	XMFLOAT4X4 m_viewportMatrix;
	XMFLOAT3 m_position;
	XMFLOAT3 m_right;
	XMFLOAT3 m_up;
	XMFLOAT3 m_look;

	struct ConstantBuffer
	{
		XMMATRIX ViewMatrix;
		XMMATRIX ProjectionMatrix;
		XMMATRIX ViewProjectionMatrix;
		XMMATRIX ViewportMatrix;
		XMFLOAT3 CameraPosition;
		float padding;
	};

	Changes m_changes;

public:
	ICamera();
	virtual ~ICamera();
	virtual void HandleEvent(GraphicsWindow::Event& event) = 0;
	void ComputeRayFromMousePointer(XMFLOAT3& origin, XMFLOAT3& direction);
	void SetLens(float fovY, float aspect, float zn, float zf);
	void OnResize(int width, int height);
	virtual void Update(float dt, bool gameUIHasFocus) = 0;
	XMMATRIX GetViewMatrix() const;
	XMMATRIX GetProjectionMatrix() const;
	XMMATRIX GetViewportMatrix() const;
	XMMATRIX GetViewProjectionMatrix() const;
	ID3D11Buffer* GetConstantBuffer() const;
	XMFLOAT3 GetPosition() const { return m_position; };
	XMVECTOR GetPositionXM() const { return XMLoadFloat3(&m_position); };
	void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& position);

};