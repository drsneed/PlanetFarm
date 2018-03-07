#pragma once
#include "ICamera.h"

class Camera : public ICamera
{
public:

	Camera();
	~Camera();

	virtual void Update(float dt, bool gameUIHasFocus) override;
	void OnMouseMove(int x, int y);
	void EnableRotateView(bool enable, int mouseX, int mouseY);


	auto GetCurrentSpeedMultiplier() const -> float;
	// Get Camera Basis Vectors
	XMVECTOR GetRightXM() const;
	XMFLOAT3 GetRight() const;
	XMVECTOR GetUpXM() const;
	XMFLOAT3 GetUp() const;
	XMVECTOR GetLookXM() const;

	// Get Frustum Properties
	float GetNearZ() const;
	float GetFarZ() const;
	float GetAspect() const;
	float GetFOVY() const;
	float GetFOVX() const;

	// Get new and far plane dimensions in view space coordinates
	float  GetNearWindowWidth() const;
	float GetNearWindowHeight() const;
	float GetFarWindowWidth() const;
	float GetFarWindowHeight() const;



	void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
	void LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& worldUp);

	void Strafe(float d);
	void Walk(float d);

	void Pitch(float angle);
	void RotateY(float angle);

	void Zoom(float d);

	void Freeze();


	virtual void HandleEvent(GraphicsWindow::Event& event) override;

private:

	void _HandleMovement(float dt);
	void _HandleMatrixUpdate();

	// cache frustum properties
	float m_nearZ;
	float m_farZ;
	float m_aspect;
	float m_fovY;
	float m_nearWindowHeight;
	float m_farWindowHeight;
	

	float m_speed;
	bool m_rotationEnabled;
	int m_lastMouseX;
	int m_lastMouseY;
	bool m_ctrlDown;

};
