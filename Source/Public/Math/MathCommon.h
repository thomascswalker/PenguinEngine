#pragma once

#include "MathFwd.h"
#include "Color.h"
#include "Matrix.h"
#include "Plane.h"
#include "Quat.h"
#include "Rect.h"
#include "Rotator.h"
#include "Transform.h"
#include "Vector.h"
#include "VectorIntrinsic.h"

namespace Math
{
	// Rotate a vector with the specified quaternion.
	// https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.inl#L18
	// https://github.com/mrdoob/three.js/blob/dev/src/math/Vector3.js#L252
	template <typename T>
	vec3_t<T> rotate(const vec3_t<T>& V, const quat_t<T> Q)
	{
		T tx = T(2) * (Q.y * V.z - Q.z * V.y);
		T ty = T(2) * (Q.z * V.x - Q.x * V.z);
		T tz = T(2) * (Q.x * V.y - Q.y * V.x);
		vec3_t<T> out;

		out.x = V.x + Q.w * tx + Q.y * tz - Q.z * ty;
		out.y = V.y + Q.w * ty + Q.z * tx - Q.x * tz;
		out.z = V.z + Q.w * tz + Q.x * ty - Q.y * tx;

		return out;
	}

	// Rotate a vector with the specified Pitch, Yaw, and Roll (in degrees)
	template <typename T>
	vec3_t<T> Rotate(const vec3_t<T>& V, T Pitch, T Yaw, T Roll)
	{
		// Convert degrees to radians
		T PitchRad = Math::degreesToRadians(Pitch);
		T YawRad = Math::degreesToRadians(Yaw);
		T RollRad = Math::degreesToRadians(Roll);

		// Construct a quaternion from euler angles
		quat_t<T> Q(PitchRad, YawRad, RollRad);

		// Rotate the vector with the quaternion and return the result
		return Rotate(V, Q);
	}

	// Rotator to Quaternion
	template <typename T>
	quat_t<T> ToQuat(rot_t<T> R)
	{
		return quat_t<T>(T(R.Pitch), T(R.Yaw), T(R.Roll));
	}

	// Quaternion to Rotator
	template <typename T>
	rot_t<T> ToRot(quat_t<T> Q)
	{
		T Roll = std::atan2f(2.0f * Q.y * Q.w - 2.0f * Q.x * Q.z, 1.0f - 2.0f * Q.y * Q.y - 2.0f * Q.z * Q.z);
		T Pitch = std::atan2f(2.0f * Q.x * Q.w - 2.0f * Q.y * Q.z, 1.0f - 2.0f * Q.x * Q.x - 2.0f * Q.z * Q.z);
		T Yaw = std::asinf(2.0f * Q.x * Q.y + 2.0f * Q.z * Q.w);

		rot_t Result(Pitch, Yaw, Roll);
		return Result;
	}

	/*
	 * Transform the specified vec4_t by the specified mat4_t.
	 */
	template <typename T>
	vec4_t<T> VectorTransformMatrix(const vec4_t<T>& V, const mat4_t<T>& M)
	{
		return vec4f{
			(V.x * M.m[0][0]) + (V.y * M.m[1][0]) + (V.z * M.m[2][0]) + (V.w * M.m[3][0]),
			(V.x * M.m[0][1]) + (V.y * M.m[1][1]) + (V.z * M.m[2][1]) + (V.w * M.m[3][1]),
			(V.x * M.m[0][2]) + (V.y * M.m[1][2]) + (V.z * M.m[2][2]) + (V.w * M.m[3][2]),
			(V.x * M.m[0][3]) + (V.y * M.m[1][3]) + (V.z * M.m[2][3]) + (V.w * M.m[3][3])
		};
	}

	template <typename T>
	vec3_t<T> ToVector(const rot_t<T>& Rot)
	{
		// Remove winding and clamp to [-360, 360]
		const T PitchNoWinding = std::clamp(std::fmodf(Rot.Pitch, static_cast<T>(360.0)), T(-360), T(360));
		const T YawNoWinding = std::clamp(std::fmodf(Rot.Yaw, static_cast<T>(360.0)), T(-360), T(360));

		T CP, SP, CY, SY;
		Math::sinCos(&SP, &CP, Math::degreesToRadians(PitchNoWinding));
		Math::sinCos(&SY, &CY, Math::degreesToRadians(YawNoWinding));

		return {CP * CY, CP * SY, SP};
	}
}
