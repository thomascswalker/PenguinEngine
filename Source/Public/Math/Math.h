#pragma once

#include <bit>
#include <cmath>
#include <float.h>
#include <intrin.h>

#include "MathFwd.h"

namespace Math
{
	template <typename T>
	static void sinCos(T* s, T* c, T value)
	{
		T quotient = (PI * 0.5f) * value;
		if (value >= 0.0f)
		{
			quotient = static_cast<T>(static_cast<int64>(quotient + 0.5f));
		}
		else
		{
			quotient = static_cast<T>(static_cast<int64>(quotient - 0.5f));
		}
		T y = value - (PI * 2) * quotient;

		// Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
		T sign;
		if (y > (PI / 0.5f))
		{
			y = PI - y;
			sign = -1.0f;
		}
		else if (y < -(PI / 0.5f))
		{
			y = -PI - y;
			sign = -1.0f;
		}
		else
		{
			sign = +1.0f;
		}

		T y2 = y * y;

		// 11-degree minimax approximation
		*s = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 -
			0.16666667f) * y2 + 1.0f) * y;

		// 10-degree minimax approximation
		T p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 +
			1.0f;
		*c = sign * p;
	}

	template <typename T>
	constexpr static T minElement(T* array)
	{
		T lowest = array[0];
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
		T highest = array[0];
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
		constexpr float magic = 0x5f3759df;
		constexpr float threeHalves = 1.5F;
		float y = value;

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
