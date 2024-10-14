#pragma once

#include "Math/Vector.h"

struct MouseData
{
	vec2f position{};
	vec2f clickPosition{};
	bool  leftDown = false;
	bool  middleDown = false;
	bool  rightDown = false;
	float middleDelta = 0.0f;

};