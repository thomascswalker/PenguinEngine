#pragma once

#include <format>
#include "MathFwd.h"

// Rotator composed of euler angles, in degrees
template <typename T>
struct rot_t
{
	// Rotation around Right axis (z)
	T Pitch = 0;
	// Rotation around Up axis (y)
	T Yaw = 0;
	// Rotation around Forward axis (x)
	T Roll = 0;

	rot_t() = default;

	rot_t(T InPitch, T InYaw, T InRoll) : Pitch(InPitch), Yaw(InYaw), Roll(InRoll)
	{
	}

	rot_t(const vec3_t<T>& euler);

	static rot_t Identity() { return rot_t(); }

	T NormalizeAxis(T Angle) const
	{
		T Remainder = std::fmodf(Angle, T(360));
		return std::fmodf(Remainder + T(360), T(360));
	}

	void Normalize()
	{
		Pitch = NormalizeAxis(Pitch);
		Yaw = NormalizeAxis(Yaw);
		Roll = NormalizeAxis(Roll);
	}

	std::string ToString() const { return std::format("[Pitch={}, Yaw={}, Roll={}]", Pitch, Yaw, Roll); }

	rot_t operator+(const rot_t& Other)
	{
		rot_t Out;
		Out.Pitch = Pitch + Other.Pitch;
		Out.Yaw = Yaw + Other.Yaw;
		Out.Roll = Roll + Other.Roll;
		return Out;
	}

	rot_t& operator+=(const rot_t& Other)
	{
		*this = *this + Other;
		return *this;
	}

	rot_t operator*(T Scale) const
	{
		return {Pitch * Scale, Yaw * Scale, Roll * Scale};
	}
};

//
// quat_t<float> rot_t<float>::Quaternion() const;
