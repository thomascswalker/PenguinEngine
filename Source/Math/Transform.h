#pragma once

#include "Core/Logging.h"
#include "Rotator.h"
#include "Vector.h"

template <typename T>
struct trans_t
{
	vec3_t<T> translation;
	rot_t<T> rotation;
	vec3_t<T> scale;

	trans_t()
	{
		rotation    = rot_t<T>(0, 0, 0);
		translation = vec3_t<T>(0);
		scale       = vec3_t<T>(1);
	}

	explicit trans_t(const vec3_t<T>& inTranslation)
	{
		rotation    = rot_t<T>();
		translation = inTranslation;
		scale       = vec3_t<T>();
	}

	explicit trans_t(rot_t<T>& inRotation)
	{
		rotation    = inRotation;
		translation = vec3_t<T>();
		scale       = vec3_t<T>();
	}

	trans_t(quat_t<T>& inRotation, const vec3_t<T>& inTranslation, const vec3_t<T>& inScale = {1.0f, 1.0f, 1.0f})
	{
		rotation    = inRotation.rotator();
		translation = inTranslation;
		scale       = inScale;
	}

	trans_t(rot_t<T>& inRotation, const vec3_t<T>& inTranslation, const vec3_t<T>& inScale = {1.0f, 1.0f, 1.0f})
	{
		rotation    = inRotation;
		translation = inTranslation;
		scale       = inScale;
	}

	void fromMatrix(mat4_t<T>& inMatrix)
	{
		// Extract scale
		scale = inMatrix.getScale();

		// m_handle negative scaling
		LOG_WARNING("Implement negative scale handling in trans_t::FromMatrix")

		// Extract rotation
		quat_t<T> inRotation = quat_t(inMatrix);
		rotation             = inRotation.rotator();

		// Extract translation
		translation = inMatrix.getTranslation();

		rotation.normalize();
	}

	//  | rx0   | rx1   | rx2   | 0 |
	//  | ry0   | ry1   | ry2   | 0 |
	//  | rz0   | rz1   | rz2   | 0 |
	//  | tx*sx | ty*sy | tz*sz | 1 | 
	mat4_t<T> toMatrix() const
	{
		// Apply translation
		mat4_t<T> out = translationMatrix(translation);

		// Apply rotation
		mat4_t rot = rotationMatrix(rotation);
		for (int32 x = 0; x < 3; ++x)
		{
			for (int32 y = 0; y < 3; ++y)
			{
				out.m[y][x] = rot.m[y][x];
			}
		}

		// Apply scale
		out.m[3][0] *= scale.x;
		out.m[3][1] *= scale.y;
		out.m[3][2] *= scale.z;

		return out;
	}

	vec3_t<T> getAxisNormalized(EAxis inAxis) const
	{
		mat4_t<T> m          = toMatrix().getInverse();
		vec3_t<T> axisVector = m.getAxis(inAxis);
		axisVector.normalize();
		return axisVector;
	}

	std::string toString() const
	{
		return std::format("translation={}, rotation={}, scale={}", translation.toString(), rotation.toString(),
		                   scale.toString());
	}

	trans_t operator*(const trans_t& other)
	{
		trans_t out;

		out.translation = translation * other.translation;
		out.rotation    = other.rotation;
		out.scale       = scale * other.scale;

		return out;
	}

	trans_t& operator*=(const trans_t& other)
	{
		*this = *this * other;
		return *this;
	}
};
