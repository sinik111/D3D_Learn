#include "Input.h"

#include <windows.h>
#include <bitset>

static std::bitset<256> s_currentKeyState;
static std::bitset<256> s_previousKeyState;

void Input::Update()
{
    s_previousKeyState = s_currentKeyState;

    for (int i = 0; i < 256; ++i)
    {
        s_currentKeyState[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
    }
}

bool Input::IsKeyHeld(int vkey)
{
    return s_currentKeyState[vkey];
}

bool Input::IsKeyPressed(int vkey)
{
    return !s_previousKeyState[vkey] && s_currentKeyState[vkey];
}

bool Input::IsKeyReleased(int vkey)
{
    return s_previousKeyState[vkey] && !s_currentKeyState[vkey];
}