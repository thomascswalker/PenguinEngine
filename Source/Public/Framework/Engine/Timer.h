#pragma once

#include <chrono>

using steadyClock = std::chrono::steady_clock;
using timePoint = std::chrono::time_point<std::chrono::steady_clock>;
using durationMs = std::chrono::duration<float, std::milli>;

struct PTimer
{
	static timePoint now()
	{
		return steadyClock::now();
	}
};
