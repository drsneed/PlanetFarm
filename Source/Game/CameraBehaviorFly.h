#pragma once
#include "CameraBehavior.h"

class CameraBehaviorFly : public CameraBehavior
{

	float m_velocity;
	int m_lastMouseX;
	int m_lastMouseY;
	void _CaptureMousePos();

public:
	CameraBehaviorFly();
	~CameraBehaviorFly();

	virtual void Tick(Camera* camera, float dt) override;
	virtual void HandleEvent(Camera* camera, GraphicsWindow::Event& event) override;

};