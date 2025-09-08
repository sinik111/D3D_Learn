#include "MyTime.h"

#include <chrono>

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

static TimePoint s_previousTime = Clock::now();
static TimePoint s_currentTime = Clock::now();

static float s_deltaTime = 0.0f;

void MyTime::Update()
{
    s_currentTime = Clock::now();

    std::chrono::duration<float> duration(s_currentTime - s_previousTime);

    s_deltaTime = duration.count();

    s_previousTime = s_currentTime;
}

float MyTime::DeltaTime()
{
    return s_deltaTime;
}