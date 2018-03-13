#pragma once
#include "CameraBehavior.h"

class CameraBehaviorMap : public CameraBehavior
{
public:
	enum class State : uint8_t
	{
		Idle,
		Panning,
		Rotating,
		PanningEaseOut,
		RotatingEaseOut
	};

private:
	XMFLOAT2 _velocity;
	XMFLOAT2 _mouse_pointer_pan_start;
	XMFLOAT2 _mouse_pointer_last;
	State _state;
	float _ease_timer;
	
	float _Distance(XMFLOAT2 a, XMFLOAT2 b)
	{
		XMVECTOR _a = XMLoadFloat2(&a);
		XMVECTOR _b = XMLoadFloat2(&b);
		FLOAT result;
		XMStoreFloat(&result, XMVector2Length(_b - _a));
		return result;
	}

	void _IdleTick(Camera* camera, float dt);
	void _PanningTick(Camera* camera, float dt);
	void _RotatingTick(Camera* camera, float dt);
	void _PanningEaseOutTick(Camera* camera, float dt);
	void _RotatingEaseOutTick(Camera* camera, float dt);
public:
	CameraBehaviorMap();
	~CameraBehaviorMap();

	virtual void Tick(Camera* camera, float dt) override;
	virtual void HandleEvent(Camera* camera, GraphicsWindow::Event& event) override;

	

};
