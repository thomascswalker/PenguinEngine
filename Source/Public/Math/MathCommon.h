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
	TVector3<T> Rotate(const TVector3<T>& V, const TQuat<T> Q)
	{
		T TX = T(2) * (Q.Y * V.Z - Q.Z * V.Y);
		T TY = T(2) * (Q.Z * V.X - Q.X * V.Z);
		T TZ = T(2) * (Q.X * V.Y - Q.Y * V.X);
		TVector3<T> Out;

		Out.X = V.X + Q.W * TX + Q.Y * TZ - Q.Z * TY;
		Out.Y = V.Y + Q.W * TY + Q.Z * TX - Q.X * TZ;
		Out.Z = V.Z + Q.W * TZ + Q.X * TY - Q.Y * TX;

		return Out;
	}

	// Rotate a vector with the specified Pitch, Yaw, and Roll (in degrees)
	template <typename T>
	TVector3<T> Rotate(const TVector3<T>& V, T Pitch, T Yaw, T Roll)
	{
		// Convert degrees to radians
		T PitchRad = Math::DegreesToRadians(Pitch);
		T YawRad = Math::DegreesToRadians(Yaw);
		T RollRad = Math::DegreesToRadians(Roll);

		// Construct a quaternion from euler angles
		TQuat<T> Q(PitchRad, YawRad, RollRad);

		// Rotate the vector with the quaternion and return the result
		return Rotate(V, Q);
	}

	// Rotator to Quaternion
	template <typename T>
	TQuat<T> ToQuat(TRotator<T> R)
	{
		return TQuat<T>(T(R.Pitch), T(R.Yaw), T(R.Roll));
	}

	// Quaternion to Rotator
	template <typename T>
	TRotator<T> ToRot(TQuat<T> Q)
	{
		T Roll = Math::ATan2(2.0f * Q.Y * Q.W - 2.0f * Q.X * Q.Z, 1.0f - 2.0f * Q.Y * Q.Y - 2.0f * Q.Z * Q.Z);
		T Pitch = Math::ATan2(2.0f * Q.X * Q.W - 2.0f * Q.Y * Q.Z, 1.0f - 2.0f * Q.X * Q.X - 2.0f * Q.Z * Q.Z);
		T Yaw = Math::ASin(2.0f * Q.X * Q.Y + 2.0f * Q.Z * Q.W);

		TRotator Result(Pitch, Yaw, Roll);
		return Result;
	}

	/*
	 * Transform the specified TVector4 by the specified TMatrix.
	 */
	template <typename T>
	TVector4<T> VectorTransformMatrix(const TVector4<T>& V, const TMatrix<T>& M)
	{
		return vec4f{
			(V.X * M.M[0][0]) + (V.Y * M.M[1][0]) + (V.Z * M.M[2][0]) + (V.W * M.M[3][0]),
			(V.X * M.M[0][1]) + (V.Y * M.M[1][1]) + (V.Z * M.M[2][1]) + (V.W * M.M[3][1]),
			(V.X * M.M[0][2]) + (V.Y * M.M[1][2]) + (V.Z * M.M[2][2]) + (V.W * M.M[3][2]),
			(V.X * M.M[0][3]) + (V.Y * M.M[1][3]) + (V.Z * M.M[2][3]) + (V.W * M.M[3][3])
		};
	}

	template <typename T>
	TVector3<T> ToVector(const TRotator<T>& Rot)
	{
		// Remove winding and clamp to [-360, 360]
		const T PitchNoWinding = Math::Clamp(std::fmodf(Rot.Pitch, static_cast<T>(360.0)), T(-360), T(360));
		const T YawNoWinding = Math::Clamp(std::fmodf(Rot.Yaw, static_cast<T>(360.0)), T(-360), T(360));

		T CP, SP, CY, SY;
		Math::SinCos(&SP, &CP, Math::DegreesToRadians(PitchNoWinding));
		Math::SinCos(&SY, &CY, Math::DegreesToRadians(YawNoWinding));

		return {CP * CY, CP * SY, SP};
	}
}
