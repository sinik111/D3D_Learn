#pragma once

#include <chrono>

namespace MyTime
{
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = std::chrono::time_point<Clock>;
	using Duration = std::chrono::duration<float>;

	void Update();

	float DeltaTime();
	TimePoint GetTimestamp();
	TimePoint GetAccumulatedTime(const TimePoint& timePoint, int seconds);
	float GetElapsedSeconds(const TimePoint& timePoint);
}