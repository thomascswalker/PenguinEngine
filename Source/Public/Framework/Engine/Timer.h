#pragma once

#include <chrono>

using SteadyClock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;
using DurationMs = std::chrono::duration<float, std::milli>;

struct PTimer
{
    static TimePoint Now()
    {
        return SteadyClock::now();
    }
};
