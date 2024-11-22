#pragma once

#include <bit>
#include <cmath>
#include <float.h>
#include <intrin.h>
#include <cassert>

#include "MathFwd.h"

namespace Math
{
	template <typename T>
	static void sinCos(T* s, T* c, T value)
	{
		assert(s);
		assert(c);

		// Map value to y in [-pi,pi], x = 2*pi*quotient + remainder.
		float quotient = g_inv2Pi * value;
		if (value >= 0.0f)
		{
			quotient = static_cast<float>(static_cast<int>(quotient + 0.5f));
		}
		else
		{
			quotient = static_cast<float>(static_cast<int>(quotient - 0.5f));
		}
		float y = value - g_2Pi * quotient;

		// Map y to [-pi/2,pi/2] with sin(y) = sin(value).
		float sign;
		if (y > g_piDiv2)
		{
			y    = g_pi - y;
			sign = -1.0f;
		}
		else if (y < -g_piDiv2)
		{
			y    = -g_pi - y;
			sign = -1.0f;
		}
		else
		{
			sign = +1.0f;
		}

		float y2 = y * y;

		// 7-degree minimax approximation
		*s = (((-0.00018524670f * y2 + 0.0083139502f) * y2 - 0.16665852f) * y2 + 1.0f) * y;

		// 6-degree minimax approximation
		float p = ((-0.0012712436f * y2 + 0.041493919f) * y2 - 0.49992746f) * y2 + 1.0f;
		*c      = sign * p;
	}

	template <typename T>
	constexpr static T minElement(T* array)
	{
		T lowest   = array[0];
		int32 size = sizeof(array) / sizeof(array[0]);
		for (int32 index = 0; index < size; index++)
		{
			if (array[index] < lowest)
			{
				lowest = array[index];
			}
		}
		return lowest;
	}

	template <typename T>
	constexpr static T maxElement(T* array)
	{
		T highest  = array[0];
		int32 size = sizeof(array) / sizeof(array[0]);
		for (int32 index = 0; index < size; index++)
		{
			if (array[index] > highest)
			{
				highest = array[index];
			}
		}
		return highest;
	}

	template <typename T>
	constexpr static T square(T value)
	{
		return value * value;
	}

	template <typename T>
	constexpr static T invSqrt(T value)
	{
		// https://en.wikipedia.org/wiki/Fast_inverse_square_root
		constexpr float magic       = 0x5f3759df;
		constexpr float threeHalves = 1.5F;
		float y                     = value;

		long i = *reinterpret_cast<long*>(&y);

		i = magic - (i >> 1);
		y = *reinterpret_cast<float*>(&i);

		y = y * (threeHalves - ((value * 0.5f) * y * y));

		return y;
	}

	template <typename T>
	constexpr static bool isFinite(T value)
	{
		return _finite(value) != 0;
	}

	template <typename T>
	constexpr static bool sign(T value)
	{
		return value >= 0;
	}

	template <typename T>
	constexpr static T degreesToRadians(T degrees)
	{
		return degrees * DEG_TO_RAD;
	}

	template <typename T>
	constexpr static T radiansToDegrees(T radians)
	{
		return radians * RAD_TO_DEG;
	}

	template <typename T>
	T remap(T value, T oldMin, T oldMax, T newMin, T newMax)
	{
		return (((value - oldMin) * (newMax - newMin)) / (oldMax - oldMin)) + newMin;
	}

	inline int32 truncate(const float value)
	{
		return static_cast<int32>(value);
	}

	inline bool closeEnough(const float a, const float b)
	{
		return std::abs(a - b) < EPSILON;
	}
};
