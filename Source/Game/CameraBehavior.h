#pragma once
#include <Core/GraphicsWindow.h>

class Camera;

class CameraBehavior
{
public:
	virtual void Tick(Camera* camera, float dt) = 0;
	virtual void HandleEvent(Camera* camera, GraphicsWindow::Event& event) = 0;
};