#pragma once
#include "ICamera.h"

class FirstPersonCamera : public ICamera
{
	float m_yaw;
	float m_pitch;
	float m_roll;
	XMFLOAT3 m_up;
	XMFLOAT3 m_look;
	XMFLOAT3 m_right;
	int m_lastMouseX;
	int m_lastMouseY;
	void _HandleMatrixUpdate();
	void _HandleMovement(float dt);
	void _ResetWindowMousePos();

public:
	FirstPersonCamera();
	~FirstPersonCamera();

	void Yaw(float amount);
	void Pitch(float amount);
	void Roll(float amount);

	virtual void Update(float dt, bool uiHasFocus) override;
	virtual void HandleEvent(GraphicsWindow::Event& event) override;

	XMVECTOR GetLookVectorXM() const { return XMLoadFloat3(&m_look); }
	XMVECTOR GetUpVectorXM() const { return XMLoadFloat3(&m_up); }
	XMVECTOR GetRightVectorXM() const { return XMLoadFloat3(&m_right); }

};