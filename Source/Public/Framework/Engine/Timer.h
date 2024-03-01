#pragma once

#include <chrono>

typedef std::chrono::steady_clock SteadyClock;
typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;
typedef std::chrono::duration<float, std::milli> DurationMs;

struct PTimer
{
    static TimePoint Now()
    {
        return SteadyClock::now();
    }
};
