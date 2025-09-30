#pragma once

#include <directxtk/SimpleMath.h>

class Camera
{
private:
	using Vector3 = DirectX::SimpleMath::Vector3;
	using Matrix = DirectX::SimpleMath::Matrix;

	Vector3 m_position{ 0.0f, 0.0f, -10.0f };
	Vector3 m_rotation{ 0.0f, 0.0f, 0.0f };
	Matrix m_world = Matrix::Identity;

	float m_fov = 50.0f;
	float m_near = 0.01f;
	float m_far = 100.0f;

	float m_speed = 10.0f;
	float m_rotateSpeed = 0.1f;

public:
	Vector3 GetForward() const;
	Vector3 GetRight() const;
	Vector3 GetPosition() const;
	Vector3 GetRotation() const;
	Matrix GetWorldMatrix() const;
	void GetViewMatrix(Matrix& out) const;
	float GetFOV() const;
	float GetNear() const;
	float GetFar() const;

	void SetPosition(const Vector3& position);
	void SetRotation(const Vector3& rotation);
	void SetFOV(float degree);
	void SetNear(float n);
	void SetFar(float f);

	void Update();
	void ProcessInput();
};