#pragma once
#include "FirstPersonCamera.h"

class Player
{
public:
	Player();
	~Player();
	std::shared_ptr<ICamera> GetCamera() const { return m_camera; }
	void Update(float dt, bool uiHasFocus);
	void HandleEvent(GraphicsWindow::Event& event);
	void EndFrame();
	void SetFlying(bool flying);
	bool IsFlying() const { return m_flying; }
private:
	float m_speed;
	bool m_flying;
	std::shared_ptr<FirstPersonCamera> m_camera;
	void _UpdatePosition(float dt);
	bool _Collision(XMVECTOR position);
	
};
