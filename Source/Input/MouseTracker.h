#pragma once

#include "Math/Vector.h"

struct MouseTracker
{
	bool isTracking = false;
	vec2f start;
	vec2f end;
	vec2f rawDelta;

	bool getIsTracking() const { return isTracking; }

	void startTracking(const vec2f& screenPosition)
	{
		isTracking = true;
		start = screenPosition;
	}

	void endTracking(const vec2f& screenPosition)
	{
		isTracking = false;
		end = screenPosition;
	}

	void addDelta(const int32 inDelta)
	{
		rawDelta += inDelta;
	}

	vec2f getDelta() const
	{
		const vec2f delta(end - start);
		return delta;
	}
};
