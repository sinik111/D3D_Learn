#include "MyTime.h"

namespace MyTime
{
    static TimePoint s_previousTime = Clock::now();
    static TimePoint s_currentTime = Clock::now();

    static float s_deltaTime = 0.0f;
}

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