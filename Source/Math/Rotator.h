#pragma once

#include <format>
#include "MathFwd.h"

// Rotator composed of euler angles, in degrees
template <typename T>
struct rot_t
{
	// rotation around Right axis (z)
	T pitch = 0;
	// rotation around Up axis (y)
	T yaw = 0;
	// rotation around Forward axis (x)
	T roll = 0;

	rot_t() = default;

	rot_t(T inPitch, T inYaw, T inRoll) : pitch(inPitch), yaw(inYaw), roll(inRoll)
	{
	}

	rot_t(const vec3_t<T>& euler);

	static rot_t identity() { return rot_t(); }

	T normalizeAxis(T angle) const
	{
		T remainder = std::fmodf(angle, T(360));
		return std::fmodf(remainder + T(360), T(360));
	}

	void normalize()
	{
		pitch = normalizeAxis(pitch);
		yaw = normalizeAxis(yaw);
		roll = normalizeAxis(roll);
	}

	std::string toString() const { return std::format("[pitch={}, yaw={}, roll={}]", pitch, yaw, roll); }

	rot_t operator+(const rot_t& other)
	{
		rot_t out;
		out.pitch = pitch + other.pitch;
		out.yaw = yaw + other.yaw;
		out.roll = roll + other.roll;
		return out;
	}

	rot_t& operator+=(const rot_t& other)
	{
		*this = *this + other;
		return *this;
	}

	rot_t operator*(T scale) const
	{
		return {pitch * scale, yaw * scale, roll * scale};
	}
};
