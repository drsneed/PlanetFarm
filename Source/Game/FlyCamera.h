#pragma once
#include "CameraBase.h"

class FlyCamera : public CameraBase
{
	float m_yaw;
	float m_pitch;
	float m_roll;
	float m_velocity;
	XMFLOAT3 m_up;
	XMFLOAT3 m_look;
	XMFLOAT3 m_right;
	int m_lastMouseX;
	int m_lastMouseY;
	void _HandleMatrixUpdate();
	void _HandleMovement(float dt);
	void _ResetWindowMousePos();

public:
	FlyCamera();
	~FlyCamera();

	void Yaw(float amount);
	void Pitch(float amount);
	void Roll(float amount);

	virtual void Tick(float dt) override;
	virtual void HandleEvent(GraphicsWindow::Event& event) override;

	XMVECTOR GetLookVectorXM() const { return XMLoadFloat3(&m_look); }
	XMVECTOR GetUpVectorXM() const { return XMLoadFloat3(&m_up); }
	XMVECTOR GetRightVectorXM() const { return XMLoadFloat3(&m_right); }

};