#pragma once

#include "Framework/Core/Logging.h"
#include "Rotator.h"
#include "Vector.h"

template <typename T>
struct trans_t
{
	vec3_t<T> Translation;
	rot_t<T> Rotation;
	vec3_t<T> Scale;

	trans_t()
	{
		Rotation = rot_t<T>(0, 0, 0);
		Translation = vec3_t<T>(0);
		Scale = vec3_t<T>(1);
	}

	explicit trans_t(const vec3_t<T>& InTranslation)
	{
		Rotation = rot_t<T>();
		Translation = InTranslation;
		Scale = vec3_t<T>();
	}

	explicit trans_t(rot_t<T>& InRotation)
	{
		Rotation = InRotation;
		Translation = vec3_t<T>();
		Scale = vec3_t<T>();
	}

	trans_t(quat_t<T>& InRotation, const vec3_t<T>& InTranslation, const vec3_t<T>& InScale = {1.0f, 1.0f, 1.0f})
	{
		Rotation = InRotation.Rotator();
		Translation = InTranslation;
		Scale = InScale;
	}

	trans_t(rot_t<T>& InRotation, const vec3_t<T>& InTranslation, const vec3_t<T>& InScale = {1.0f, 1.0f, 1.0f})
	{
		Rotation = InRotation;
		Translation = InTranslation;
		Scale = InScale;
	}

	void FromMatrix(mat4_t<T>& InMatrix)
	{
		// Extract scale
		Scale = InMatrix.GetScale();

		// Handle negative scaling
		LOG_WARNING("Implement negative scale handling in trans_t::FromMatrix")

		// Extract rotation
		quat_t<T> InRotation = quat_t(InMatrix);
		Rotation = InRotation.Rotator();

		// Extract translation
		Translation = InMatrix.GetTranslation();

		Rotation.Normalize();
	}

	//  | rx0   | rx1   | rx2   | 0 |
	//  | ry0   | ry1   | ry2   | 0 |
	//  | rz0   | rz1   | rz2   | 0 |
	//  | tx*sx | ty*sy | tz*sz | 1 | 
	mat4_t<T> ToMatrix() const
	{
		// Apply translation
		mat4_t<T> Out = mat4_trans_t<T>(Translation);

		// Apply rotation
		mat4_t RotationMatrix = mat4_rot_t<T>(Rotation);
		for (int32 X = 0; X < 3; ++X)
		{
			for (int32 Y = 0; Y < 3; ++Y)
			{
				Out.m[Y][X] = RotationMatrix.m[Y][X];
			}
		}

		// Apply scale
		Out.m[3][0] *= Scale.x;
		Out.m[3][1] *= Scale.y;
		Out.m[3][2] *= Scale.z;

		return Out;
	}

	vec3_t<T> GetAxisNormalized(EAxis InAxis) const
	{
		mat4_t<T> M = ToMatrix().GetInverse();
		vec3_t<T> AxisVector = M.GetAxis(InAxis);
		AxisVector.Normalize();
		return AxisVector;
	}

	std::string ToString() const
	{
		return std::format("Translation={}, Rotation={}, Scale={}", Translation.ToString(), Rotation.ToString(),
		                   Scale.ToString());
	}

	trans_t operator*(const trans_t& Other)
	{
		trans_t Out;

		Out.Translation = Translation * Other.Translation;
		Out.Rotation = Other.Rotation;
		Out.Scale = Scale * Other.Scale;

		return Out;
	}

	trans_t& operator*=(const trans_t& Other)
	{
		*this = *this * Other;
		return *this;
	}
};
