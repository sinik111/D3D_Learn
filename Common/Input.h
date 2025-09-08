#pragma once

namespace Input
{
	void Update();

	bool IsKeyHeld(int vkey);
	bool IsKeyPressed(int vkey);
	bool IsKeyReleased(int vkey);
}