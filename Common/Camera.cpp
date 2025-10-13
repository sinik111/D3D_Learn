#include "Camera.h"

#include "Input.h"
#include "MyTime.h"

using namespace DirectX::SimpleMath;

Vector3 Camera::GetForward() const
{
	return -m_world.Forward();
}

Vector3 Camera::GetRight() const
{
	return m_world.Right();
}

Vector3 Camera::GetPosition() const
{
	return m_position;
}

Vector3 Camera::GetRotation() const
{
	return m_rotation;
}

Matrix Camera::GetWorldMatrix() const
{
	return m_world;
}

void Camera::GetViewMatrix(Matrix& out) const
{
	Vector3 eye = m_world.Translation();
	Vector3 to = -m_world.Forward();
	Vector3 up = m_world.Up();

	out = DirectX::XMMatrixLookToLH(eye, to, up);
}

float Camera::GetFOV() const
{
	return m_fov;
}

float Camera::GetNear() const
{
	return m_near;
}

float Camera::GetFar() const
{
	return m_far;
}

float Camera::GetSpeed() const
{
	return m_speed;
}

void Camera::SetPosition(const Vector3& position)
{
	m_position = position;
}

void Camera::SetRotation(const Vector3& rotation)
{
	m_rotation = rotation;
}

void Camera::SetFOV(float degree)
{
	m_fov = degree;
}

void Camera::SetNear(float n)
{
	m_near = n;
}

void Camera::SetFar(float f)
{
	m_far = f;
}

void Camera::SetSpeed(float speed)
{
	m_speed = speed;
}

void Camera::Update()
{
	ProcessInput();

	m_world = Matrix::CreateFromYawPitchRoll(m_rotation) *
		Matrix::CreateTranslation(m_position);
}

void Camera::ProcessInput()
{
	using namespace DirectX;
	using Keys = DirectX::Keyboard::Keys;

	const Vector3 forward = -m_world.Forward();
	const Vector3 right = m_world.Right();
	const Vector3 up = m_world.Up();

	Vector3 inputVector{ 0.0f, 0.0f, 0.0f };

	if (Input::IsKeyHeld(Keys::W))
	{
		inputVector += forward;
	}
	else if (Input::IsKeyHeld(Keys::S))
	{
		inputVector -= forward;
	}

	if (Input::IsKeyHeld(Keys::D))
	{
		inputVector += right;
	}
	else if (Input::IsKeyHeld(Keys::A))
	{
		inputVector -= right;
	}

	if (Input::IsKeyHeld(Keys::E))
	{
		inputVector += up;
	}
	else if (Input::IsKeyHeld(Keys::Q))
	{
		inputVector -= up;
	}

	inputVector.Normalize();

	float speed = m_speed;

	if (Input::IsKeyHeld(Keys::LeftShift))
	{
		speed = m_speed * 2;
	}

	m_position += inputVector * speed * MyTime::DeltaTime();

	if (Input::IsMouseHeld(Input::Button::RIGHT))
	{
		Input::SetMouseMode(Mouse::Mode::MODE_RELATIVE);

		Vector2 mouseDelta = Input::GetMouseDelta();

		m_rotation.x += XMConvertToRadians(mouseDelta.y * m_rotateSpeed);
		m_rotation.y += XMConvertToRadians(mouseDelta.x * m_rotateSpeed);
	}
	else
	{
		Input::SetMouseMode(Mouse::Mode::MODE_ABSOLUTE);
	}
}