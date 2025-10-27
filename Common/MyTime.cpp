#include "MyTime.h"

namespace
{
    MyTime::TimePoint g_previousTime = MyTime::Clock::now();
    MyTime::TimePoint g_currentTime = MyTime::Clock::now();

    float g_deltaTime = 0.0f;
}

void MyTime::Update()
{
    g_currentTime = Clock::now();

    std::chrono::duration<float> duration(g_currentTime - g_previousTime);

    g_deltaTime = duration.count();

    g_previousTime = g_currentTime;
}

float MyTime::DeltaTime()
{
    return g_deltaTime;
}

MyTime::TimePoint MyTime::GetTimestamp()
{
    return Clock::now();
}

MyTime::TimePoint MyTime::GetAccumulatedTime(const TimePoint& timePoint, int seconds)
{
    return timePoint + std::chrono::seconds(seconds);
}

float MyTime::GetElapsedSeconds(const TimePoint& timePoint)
{
    return Duration(Clock::now() - timePoint).count();
}