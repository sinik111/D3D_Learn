#pragma once

#include <directxtk/SimpleMath.h>
#include <directXTK/Mouse.h>
#include <directXTK/Keyboard.h>

namespace Input
{
	enum Button
	{
		LEFT,
		RIGHT,
		MIDDLE,
		SIDE_FRONT,
		SIDE_BACK
	};

	void Initialize(HWND hWnd);
	void Update();

	bool IsKeyHeld(DirectX::Keyboard::Keys key);
	bool IsKeyPressed(DirectX::Keyboard::Keys key);
	bool IsKeyReleased(DirectX::Keyboard::Keys key);

	bool IsMouseHeld(Input::Button button);
	bool IsMousePressed(Input::Button button);
	bool IsMouseReleased(Input::Button button);

	void SetMouseMode(DirectX::Mouse::Mode mode);
	DirectX::SimpleMath::Vector2 GetMouseDelta();
	DirectX::SimpleMath::Vector2 GetMousePosition();
}