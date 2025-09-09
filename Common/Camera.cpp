#include "Camera.h"

using namespace DirectX::SimpleMath;

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

void Camera::Update()
{
	m_world = Matrix::CreateFromYawPitchRoll(m_rotation) *
		Matrix::CreateTranslation(m_position);
}
