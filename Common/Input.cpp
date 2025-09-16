#include "Input.h"

#include <memory>
#include <cassert>

static std::unique_ptr<DirectX::Keyboard> s_keyboard = std::make_unique<DirectX::Keyboard>();
static std::unique_ptr<DirectX::Mouse> s_mouse = std::make_unique<DirectX::Mouse>();

static DirectX::Keyboard::State s_keyboardState;
static DirectX::Keyboard::KeyboardStateTracker s_keyboardStateTracker;

static DirectX::Mouse::State s_mouseState;
static DirectX::Mouse::ButtonStateTracker s_mouseStateTracker;

void Input::Initialize(HWND hWnd)
{
    s_mouse->SetWindow(hWnd);
}

void Input::Update()
{
    s_keyboardState = s_keyboard->GetState();
    s_keyboardStateTracker.Update(s_keyboardState);

    s_mouseState = s_mouse->GetState();
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
    switch (button)
    {
    case Button::LEFT:
        return s_mouseState.leftButton;

    case Button::RIGHT:
        return s_mouseState.rightButton;

    case Button::MIDDLE:
        return s_mouseState.middleButton;

    case Button::SIDE_FRONT:
        return s_mouseState.xButton1;

    case Button::SIDE_BACK:
        return s_mouseState.xButton2;

    default:
        assert(false && "wrong button");

        return false;
    }

    return false;
}

bool Input::IsMousePressed(Input::Button button)
{
    using ButtonState = DirectX::Mouse::ButtonStateTracker::ButtonState;

    switch (button)
    {
    case Button::LEFT:
        return s_mouseStateTracker.leftButton == ButtonState::PRESSED;

    case Button::RIGHT:
        return s_mouseStateTracker.rightButton == ButtonState::PRESSED;

    case Button::MIDDLE:
        return s_mouseStateTracker.middleButton == ButtonState::PRESSED;

    case Button::SIDE_FRONT:
        return s_mouseStateTracker.xButton1 == ButtonState::PRESSED;

    case Button::SIDE_BACK:
        return s_mouseStateTracker.xButton2 == ButtonState::PRESSED;

    default:
        assert(false && "wrong button");

        return false;
    }

    return false;
}

bool Input::IsMouseReleased(Input::Button button)
{
    using ButtonState = DirectX::Mouse::ButtonStateTracker::ButtonState;

    switch (button)
    {
    case Button::LEFT:
        return s_mouseStateTracker.leftButton == ButtonState::RELEASED;

    case Button::RIGHT:
        return s_mouseStateTracker.rightButton == ButtonState::RELEASED;

    case Button::MIDDLE:
        return s_mouseStateTracker.middleButton == ButtonState::RELEASED;

    case Button::SIDE_FRONT:
        return s_mouseStateTracker.xButton1 == ButtonState::RELEASED;

    case Button::SIDE_BACK:
        return s_mouseStateTracker.xButton2 == ButtonState::RELEASED;

    default:
        assert(false && "wrong button");

        return false;
    }

    return false;
}

void Input::SetMouseMode(DirectX::Mouse::Mode mode)
{
    s_mouse->SetMode(mode);
}

DirectX::SimpleMath::Vector2 Input::GetMouseDelta()
{
    if (s_mouseState.positionMode == DirectX::Mouse::MODE_RELATIVE)
    {
        return { static_cast<float>(s_mouseState.x), static_cast<float>(s_mouseState.y) };
    }

    return { 0, 0 };
}

DirectX::SimpleMath::Vector2 Input::GetMousePosition()
{
    if (s_mouseState.positionMode == DirectX::Mouse::MODE_ABSOLUTE)
    {
        return { static_cast<float>(s_mouseState.x), static_cast<float>(s_mouseState.y) };
    }

    return { 0, 0 };
}
