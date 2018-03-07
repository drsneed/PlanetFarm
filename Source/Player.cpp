#include "Player.h"

#define GRAVITY -10.f

Player::Player()
	: m_camera(std::make_shared<FirstPersonCamera>())
	, m_speed(20.f)
	, m_flying(true)
{

}

Player::~Player()
{
}

void Player::_UpdatePosition(float dt)
{
	auto window = GraphicsWindow::GetInstance();
	auto position = m_camera->GetPositionXM();
	auto look = m_camera->GetLookVectorXM();
	auto right = m_camera->GetRightVectorXM();
	auto up = m_camera->GetUpVectorXM();
	auto distance = m_speed * dt;
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

	if(moved)
	{
		XMFLOAT3 newPosition;
		XMStoreFloat3(&newPosition, position);
		m_camera->SetPosition(newPosition);
	}



/*	if(m_flying)
	{
		if (moved)
		{
			if(!_Collision(position))
			{
				XMStoreFloat3(&newPosition, position);
				m_camera->SetPosition(newPosition);
			}
			m_camera->GetChanges().Position = true;
		}
	}
	else
	{
		XMFLOAT3 gravityVector(0.0f, GRAVITY * dt, 0.0f);
		auto gravitizedPosition = position + XMLoadFloat3(&gravityVector);

		if (!_Collision(gravitizedPosition))
		{
			XMStoreFloat3(&newPosition, gravitizedPosition);
			m_camera->SetPosition(newPosition);
		}
		else if (moved && !_Collision(position))
		{
			XMStoreFloat3(&newPosition, position);
			m_camera->SetPosition(newPosition);
		}
	}*/

}

bool Player::_Collision(XMVECTOR position)
{
	return false;//m_chunkManager->Collision(position);
}

void Player::Update(float dt, bool uiHasFocus)
{
	if(!uiHasFocus)
	{
		_UpdatePosition(dt);
	}
	m_camera->Update(dt, uiHasFocus);
}

void Player::HandleEvent(GraphicsWindow::Event& event)
{
	if (event.type == GraphicsWindow::Event::Type::KeyRelease &&
		event.code == GraphicsWindow::Event::Code::Space)
	{
		//m_flying = !m_flying;
	}
	m_camera->HandleEvent(event);
}

void Player::EndFrame()
{
	m_camera->GetChanges().Reset();
}

void Player::SetFlying(bool flying)
{
	m_flying = flying;
}
