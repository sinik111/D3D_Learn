#include "Input.h"

#include <memory>

namespace Input
{
    using ButtonState = DirectX::Mouse::ButtonStateTracker::ButtonState;

    static DirectX::Keyboard s_keyboard;
    static DirectX::Mouse s_mouse;

    static DirectX::Keyboard::State s_keyboardState;
    static DirectX::Keyboard::KeyboardStateTracker s_keyboardStateTracker;

    static DirectX::Mouse::State s_mouseState;
    static DirectX::Mouse::ButtonStateTracker s_mouseStateTracker;

    static bool* s_mouseHeldStateTable[static_cast<size_t>(Input::Button::MAX)];
    static ButtonState* s_mouseStateTable[static_cast<size_t>(Input::Button::MAX)];
}

void Input::Initialize(HWND hWnd)
{
    s_mouse.SetWindow(hWnd);
    
    s_mouseHeldStateTable[static_cast<size_t>(Input::Button::LEFT)] = &s_mouseState.leftButton;
    s_mouseHeldStateTable[static_cast<size_t>(Input::Button::RIGHT)] = &s_mouseState.rightButton;
    s_mouseHeldStateTable[static_cast<size_t>(Input::Button::MIDDLE)] = &s_mouseState.middleButton;
    s_mouseHeldStateTable[static_cast<size_t>(Input::Button::SIDE_FRONT)] = &s_mouseState.xButton1;
    s_mouseHeldStateTable[static_cast<size_t>(Input::Button::SIDE_BACK)] = &s_mouseState.xButton2;

    s_mouseStateTable[static_cast<size_t>(Input::Button::LEFT)] = &s_mouseStateTracker.leftButton;
    s_mouseStateTable[static_cast<size_t>(Input::Button::RIGHT)] = &s_mouseStateTracker.rightButton;
    s_mouseStateTable[static_cast<size_t>(Input::Button::MIDDLE)] = &s_mouseStateTracker.middleButton;
    s_mouseStateTable[static_cast<size_t>(Input::Button::SIDE_FRONT)] = &s_mouseStateTracker.xButton1;
    s_mouseStateTable[static_cast<size_t>(Input::Button::SIDE_BACK)] = &s_mouseStateTracker.xButton2;
}

void Input::Update()
{
    s_keyboardState = s_keyboard.GetState();
    s_keyboardStateTracker.Update(s_keyboardState);

    s_mouseState = s_mouse.GetState();
    s_mouseStateTracker.Update(s_mouseState);
}

bool Input::IsKeyHeld(DirectX::Keyboard::Keys key)
{
    return s_keyboardState.IsKeyDown(key);
}

bool Input::IsKeyPressed(DirectX::Keyboard::Keys key)
{
    return s_keyboardStateTracker.IsKeyPressed(key);
}

bool Input::IsKeyReleased(DirectX::Keyboard::Keys key)
{
    return s_keyboardStateTracker.IsKeyReleased(key);
}

bool Input::IsMouseHeld(Input::Button button)
{
    return *s_mouseHeldStateTable[static_cast<size_t>(button)];
}

bool Input::IsMousePressed(Input::Button button)
{
    return *s_mouseStateTable[static_cast<size_t>(button)] == ButtonState::PRESSED;
}

bool Input::IsMouseReleased(Input::Button button)
{
    return *s_mouseStateTable[static_cast<size_t>(button)] == ButtonState::RELEASED;
}

void Input::SetMouseMode(DirectX::Mouse::Mode mode)
{
    s_mouse.SetMode(mode);
}

DirectX::SimpleMath::Vector2 Input::GetMouseDelta()
{
    if (s_mouseState.positionMode == DirectX::Mouse::MODE_RELATIVE)
    {
        return { static_cast<float>(s_mouseState.x), static_cast<float>(s_mouseState.y) };
    }

    return { 0.0f, 0.0f };
}

DirectX::SimpleMath::Vector2 Input::GetMousePosition()
{
    if (s_mouseState.positionMode == DirectX::Mouse::MODE_ABSOLUTE)
    {
        return { static_cast<float>(s_mouseState.x), static_cast<float>(s_mouseState.y) };
    }

    return { 0.0f, 0.0f };
}
