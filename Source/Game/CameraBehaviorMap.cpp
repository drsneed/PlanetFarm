#include "CameraBehaviorMap.h"
#include "Camera.h"

#define DEFAULT_VELOCITY 320.0f
// Minimum distance camera must move in one pan session to trigger an ease-out effect
#define EASE_OUT_MIN_DISTANCE_TRIGGER 20.0f 

// Maximum time allotted to move the distance above to trigger an ease-out effect
#define EASE_OUT_MAX_TIME_TRIGGER 0.5f

CameraBehaviorMap::CameraBehaviorMap()
	: _velocity(0.0f, 0.0f)
	, _mouse_pointer_pan_start(0.0f, 0.0f)
	, _ease_timer(0.0f)
	, _state(State::Idle)
{
	_mouse_pointer_last = GraphicsWindow::GetInstance()->GetMousePosition();
}

CameraBehaviorMap::~CameraBehaviorMap()
{
}

void CameraBehaviorMap::_IdleTick(Camera* camera, float dt)
{
	auto window = GraphicsWindow::GetInstance();
	auto position = camera->GetPositionXM();
	auto right = camera->GetRightVectorXM();
	auto up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	bool moved = false;
	if (window->IsKeyDown('W'))
	{
		auto direction = XMVector3Cross(right, up);
		position += direction * DEFAULT_VELOCITY * dt;
		moved = true;
	}
	if (window->IsKeyDown('S'))
	{
		auto real_up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		auto direction = XMVector3Cross(right, up);
		position += direction * -DEFAULT_VELOCITY * dt;
		moved = true;
	}
	if (window->IsKeyDown('A'))
	{
		position += right * -DEFAULT_VELOCITY * dt;
		moved = true;
	}
	if (window->IsKeyDown('D'))
	{
		position += right * DEFAULT_VELOCITY * dt;
		moved = true;
	}
	if (window->IsKeyDown('Q'))
	{
		position += up * DEFAULT_VELOCITY * dt * 28;
		moved = true;
	}
	if (window->IsKeyDown('E'))
	{
		position += up * -DEFAULT_VELOCITY * dt * 28;
		moved = true;
	}

	if (moved)
	{
		XMFLOAT3 newPosition;
		XMStoreFloat3(&newPosition, position);
		camera->SetPosition(newPosition);
	}
}

void CameraBehaviorMap::_PanningTick(Camera* camera, float dt)
{
	_ease_timer += dt;
}
void CameraBehaviorMap::_RotatingTick(Camera* camera, float dt)
{

}
void CameraBehaviorMap::_PanningEaseOutTick(Camera* camera, float dt)
{
	auto acceleration = XMFLOAT2(_velocity.x * dt, _velocity.y * dt);
	_ease_timer -= dt;
	_velocity.x -= acceleration.x * 4.0f;
	_velocity.y -= acceleration.y * 4.0f;
	auto old_pos = camera->GetPosition();
	camera->SetPosition(XMFLOAT3(old_pos.x + _velocity.x, old_pos.y, old_pos.z - _velocity.y));

	if (_ease_timer <= 0.0f)
	{
		_state = State::Idle;
	}
}
void CameraBehaviorMap::_RotatingEaseOutTick(Camera* camera, float dt)
{

}


void CameraBehaviorMap::Tick(Camera* camera, float dt)
{
	switch (_state)
	{
	case State::Idle:
		_IdleTick(camera, dt);
	case State::Panning:
		_PanningTick(camera, dt);
		break;
	case State::Rotating:
		_RotatingTick(camera, dt);
		break;
	case State::PanningEaseOut:
		_PanningEaseOutTick(camera, dt);
		break;
	case State::RotatingEaseOut:
		_RotatingEaseOutTick(camera, dt);
		break;
	}

	_mouse_pointer_last = GraphicsWindow::GetInstance()->GetMousePosition();
	
}

void CameraBehaviorMap::HandleEvent(Camera* camera, GraphicsWindow::Event& event)
{
	switch (event.type)
	{
	case GraphicsWindow::Event::Type::MouseClick:
		if (event.code == GraphicsWindow::Event::Code::MouseLeft)
		{
			_state = State::Panning;
			_mouse_pointer_pan_start.x = static_cast<float>(event.x);
			_mouse_pointer_pan_start.y = static_cast<float>(event.y);
			_ease_timer = 0.0f;
		}
		break;
	case GraphicsWindow::Event::Type::MouseRelease:
		if (event.code == GraphicsWindow::Event::Code::MouseLeft)
		{
			XMFLOAT2 pan_end(static_cast<float>(event.x), static_cast<float>(event.y));
			XMVECTOR from = XMLoadFloat2(&_mouse_pointer_pan_start);
			XMVECTOR to = XMLoadFloat2(&pan_end);
			XMVECTOR difference = from - to;
			XMVECTOR distance = XMVector2Length(difference);
			
			FLOAT dist;
			XMStoreFloat(&dist, distance);
			XMFLOAT2 dif;
			XMStoreFloat2(&dif, difference);

			if (dist >= EASE_OUT_MIN_DISTANCE_TRIGGER && _ease_timer <= EASE_OUT_MAX_TIME_TRIGGER)
			{
				_state = State::PanningEaseOut;
				_velocity.x = (dif.x / _ease_timer) * 0.005f;
				_velocity.y = (dif.y / _ease_timer) * 0.005f;
				_ease_timer = 1.0f;
			}
			else
			{
				_state = State::Idle;
			}
		}
		break;
	case GraphicsWindow::Event::Type::MouseMotion:
		switch (_state)
		{
		case CameraBehaviorMap::State::Panning:
		{
			XMFLOAT2 mouse_pos(static_cast<float>(event.x), static_cast<float>(event.y));
			XMVECTOR current_pos = XMLoadFloat2(&mouse_pos);
			XMVECTOR last_pos = XMLoadFloat2(&_mouse_pointer_last);
			XMVECTOR dif = last_pos - current_pos;
			XMFLOAT2 difference;
			XMStoreFloat2(&difference, dif);

			if (difference.x != 0.0 || difference.y != 0.0)
			{
				auto new_pos = camera->GetPosition();
				new_pos.x += difference.x;
				new_pos.z -= difference.y;
				camera->SetPosition(new_pos);
			}
			if (_ease_timer > 0.1f)
			{
				_mouse_pointer_pan_start = mouse_pos;
				_ease_timer = 0.0f;

			}
			break;
		}


		case CameraBehaviorMap::State::Rotating:
			break;
		}
		break;
	case GraphicsWindow::Event::Type::MouseWheelDown:
		break;
	case GraphicsWindow::Event::Type::MouseWheelUp:
		break;

	default:
		break;
	}
}