#pragma once

#include <algorithm>

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
	vec3_t<T> rotate(const vec3_t<T>& v, const quat_t<T> q)
	{
		T tx = T(2) * (q.y * v.z - q.z * v.y);
		T ty = T(2) * (q.z * v.x - q.x * v.z);
		T tz = T(2) * (q.x * v.y - q.y * v.x);
		vec3_t<T> out;

		out.x = v.x + q.w * tx + q.y * tz - q.z * ty;
		out.y = v.y + q.w * ty + q.z * tx - q.x * tz;
		out.z = v.z + q.w * tz + q.x * ty - q.y * tx;

		return out;
	}

	// Rotate a vector with the specified pitch, yaw, and roll (in degrees)
	template <typename T>
	vec3_t<T> rotate(const vec3_t<T>& v, T pitch, T yaw, T roll)
	{
		// Convert degrees to radians
		T pitchRad = Math::degreesToRadians(pitch);
		T yawRad = Math::degreesToRadians(yaw);
		T rollRad = Math::degreesToRadians(roll);

		// Construct a quaternion from euler angles
		quat_t<T> q(pitchRad, yawRad, rollRad);

		// Rotate the vector with the quaternion and return the result
		return rotate(v, q);
	}

	// Rotator to Quaternion
	template <typename T>
	quat_t<T> toQuat(rot_t<T> r)
	{
		return quat_t<T>(T(r.pitch), T(r.yaw), T(r.roll));
	}

	// Quaternion to Rotator
	template <typename T>
	rot_t<T> toRot(quat_t<T> q)
	{
		T roll = std::atan2f(2.0f * q.y * q.w - 2.0f * q.x * q.z, 1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z);
		T pitch = std::atan2f(2.0f * q.x * q.w - 2.0f * q.y * q.z, 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z);
		T yaw = std::asinf(2.0f * q.x * q.y + 2.0f * q.z * q.w);

		rot_t result(pitch, yaw, roll);
		return result;
	}

	/*
	 * Transform the specified vec4_t by the specified mat4_t.
	 */
	template <typename T>
	vec4_t<T> vectorTransformMatrix(const vec4_t<T>& v, const mat4_t<T>& m)
	{
		return vec4f{
			(v.x * m.m[0][0]) + (v.y * m.m[1][0]) + (v.z * m.m[2][0]) + (v.w * m.m[3][0]),
			(v.x * m.m[0][1]) + (v.y * m.m[1][1]) + (v.z * m.m[2][1]) + (v.w * m.m[3][1]),
			(v.x * m.m[0][2]) + (v.y * m.m[1][2]) + (v.z * m.m[2][2]) + (v.w * m.m[3][2]),
			(v.x * m.m[0][3]) + (v.y * m.m[1][3]) + (v.z * m.m[2][3]) + (v.w * m.m[3][3])
		};
	}

	template <typename T>
	vec3_t<T> toVector(const rot_t<T>& rot)
	{
		// remove winding and clamp to [-360, 360]
		const T pitchNoWinding = std::clamp(std::fmodf(rot.pitch, static_cast<T>(360.0)), T(-360), T(360));
		const T yawNoWinding = std::clamp(std::fmodf(rot.yaw, static_cast<T>(360.0)), T(-360), T(360));

		T cp, sp, cy, sy;
		Math::sinCos(&sp, &cp, Math::degreesToRadians(pitchNoWinding));
		Math::sinCos(&sy, &cy, Math::degreesToRadians(yawNoWinding));

		return {cp * cy, cp * sy, sp};
	}
}
