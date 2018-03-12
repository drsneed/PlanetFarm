#include "CameraBehaviorFly.h"
#include "Camera.h"

#define DEFAULT_VELOCITY 20.0f

CameraBehaviorFly::CameraBehaviorFly()
	: m_velocity(DEFAULT_VELOCITY)
	, m_lastMouseX(0)
	, m_lastMouseY(0)
{
	_CaptureMousePos();
}

CameraBehaviorFly::~CameraBehaviorFly()
{
}

void CameraBehaviorFly::Tick(Camera* camera, float dt)
{
	auto window = GraphicsWindow::GetInstance();
	auto position = camera->GetPositionXM();
	auto look = camera->GetLookVectorXM();
	auto right = camera->GetRightVectorXM();
	auto up = camera->GetUpVectorXM();
	auto distance = m_velocity * dt;
	bool moved = false;
	if (window->IsKeyDown('W'))
	{
		position += look * distance;
		moved = true;
	}
	if (window->IsKeyDown('S'))
	{
		position += look * -distance;
		moved = true;
	}
	if (window->IsKeyDown('A'))
	{
		position += right * -distance;
		moved = true;
	}
	if (window->IsKeyDown('D'))
	{
		position += right * distance;
		moved = true;
	}
	if (window->IsKeyDown('Q'))
	{
		position += up * distance;
		moved = true;
	}
	if (window->IsKeyDown('E'))
	{
		position += up * -distance;
		moved = true;
	}

	if (moved)
	{
		XMFLOAT3 newPosition;
		XMStoreFloat3(&newPosition, position);
		camera->SetPosition(newPosition);
	}
}

void CameraBehaviorFly::_CaptureMousePos()
{
	auto window = GraphicsWindow::GetInstance();
	window->GetMousePosition(m_lastMouseX, m_lastMouseY);
}

void CameraBehaviorFly::HandleEvent(Camera* camera, GraphicsWindow::Event& event)
{
	static bool ctrl = false;

	switch (event.type)
	{
	case GraphicsWindow::Event::Type::MouseClick:
		if (event.code == GraphicsWindow::Event::Code::MouseLeft)
		{
			ctrl = true;
			m_lastMouseX = event.x;
			m_lastMouseY = event.y;
		}
		break;
	case GraphicsWindow::Event::Type::MouseRelease:
		if (ctrl && event.code == GraphicsWindow::Event::Code::MouseLeft)
		{
			ctrl = false;
		}
		break;
	case GraphicsWindow::Event::Type::MouseMotion:
	{
		
		if(ctrl)
		{
			auto dx = XMConvertToRadians(0.25f * static_cast<float>(event.x - m_lastMouseX));
			auto dy = XMConvertToRadians(0.25f * static_cast<float>(event.y - m_lastMouseY));
			camera->Pitch(dy);
			camera->Yaw(dx);
			_CaptureMousePos();
		}

		break;
	}
	default: 
		break;
	}
}