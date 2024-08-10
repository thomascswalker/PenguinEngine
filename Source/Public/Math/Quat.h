#pragma once

template <typename T>
struct TQuat
{
	union
	{
		struct // NOLINT
		{
			T X;
			T Y;
			T Z;
			T W;
		};

		T XYZW[4];
	};

	TQuat() : X(0), Y(0), Z(0), W(0)
	{
	}

	TQuat(T InX, T InY, T InZ, T InW) : X(InX), Y(InY), Z(InZ), W(InW)
	{
	}

	// Quaternion constructed from euler angles specified (in radians)
	TQuat(const TVector3<T>& EulerAngles)
	{
		TVector3<T> C = Math::Cos(EulerAngles * T(0.5f));
		TVector3<T> S = Math::Sin(EulerAngles * T(0.5f));

		W = C.X * C.Y * C.Z + S.X * S.Y * S.Z;
		X = S.X * C.Y * C.Z - C.X * S.Y * S.Z;
		Y = C.X * S.Y * C.Z + S.X * C.Y * S.Z;
		Z = C.X * C.Y * S.Z - S.X * S.Y * C.Z;
	}

	TQuat(TVector3<T> Axis, T Angle)
	{
		const T HalfAngle = 0.5f * Angle;
		T S, C;
		Math::SinCos(&S, &C, HalfAngle);

		X = S * Axis.X;
		Y = S * Axis.Y;
		Z = S * Axis.Z;
		W = C;
	}

	TQuat(TVector3<T> From, TVector3<T> To)
	{
		T R = From.Dot(To) + T(1);
		if (R < EPSILON)
		{
			R = 0;
			if (Math::Abs(From.X) > Math::Abs(From.Z))
			{
				X = -From.Y;
				Y = From.X;
				Z = 0;
				W = R;
			}
			else
			{
				X = 0;
				Y = -From.Z;
				Z = From.Y;
				W = R;
			}
		}
		else
		{
			X = From.Y * To.Z - From.Z * To.Y;
			Y = From.Z * To.X - From.X * To.Z;
			Z = From.X * To.Y - From.Y * To.X;
			W = R;
		}
		Normalize();
	}

	TQuat(const TMatrix<T>& M)
	{
		//const MeReal *const t = (MeReal *) tm;
		T s;

		// Check diagonal (trace)
		const T tr = M.M[0][0] + M.M[1][1] + M.M[2][2];

		if (tr > 0.0f)
		{
			T InvS = Math::InvSqrt(tr + 1.f);
			this->W = 0.5f * (1.f / InvS);
			s = 0.5f * InvS;

			this->X = ((M.M[1][2] - M.M[2][1]) * s);
			this->Y = ((M.M[2][0] - M.M[0][2]) * s);
			this->Z = ((M.M[0][1] - M.M[1][0]) * s);
		}
		else
		{
			// diagonal is negative
			int32 i = 0;

			if (M.M[1][1] > M.M[0][0])
				i = 1;

			if (M.M[2][2] > M.M[i][i])
				i = 2;

			static constexpr int32 nxt[3] = {1, 2, 0};
			const int32 j = nxt[i];
			const int32 k = nxt[j];

			s = M.M[i][i] - M.M[j][j] - M.M[k][k] + 1.0f;

			T InvS = Math::InvSqrt(s);

			T qt[4];
			qt[i] = 0.5f * (1.f / InvS);

			s = 0.5f * InvS;

			qt[3] = (M.M[j][k] - M.M[k][j]) * s;
			qt[j] = (M.M[i][j] + M.M[j][i]) * s;
			qt[k] = (M.M[i][k] + M.M[k][i]) * s;

			this->X = qt[0];
			this->Y = qt[1];
			this->Z = qt[2];
			this->W = qt[3];
		}
	}

	// From Euler angles
	TQuat(const float Pitch, const float Yaw, const float Roll)
	{
		T CY = cosf(Yaw * 0.5f);
		T SY = sinf(Yaw * 0.5f);
		T CP = cosf(Pitch * 0.5f);
		T SP = sinf(Pitch * 0.5f);
		T CR = cosf(Roll * 0.5f);
		T SR = sinf(Roll * 0.5f);

		W = CR * CP * CY + SR * SP * SY;
		X = SR * CP * CY - CR * SP * SY;
		Y = CR * SP * CY + SR * CP * SY;
		Z = CR * CP * SY - SR * SP * CY;
	}

	TQuat operator*(const TQuat& Other) const
	{
		T TempW = W * Other.W - X * Other.X - Y * Other.Y - Z * Other.Z;
		T TempX = W * Other.X + X * Other.W + Y * Other.Z - Z * Other.Y;
		T TempY = W * Other.Y - X * Other.Y + Y * Other.W + Z * Other.X;
		T TempZ = W * Other.Z + X * Other.Z - Y * Other.X + Z * Other.W;

		return {TempX, TempY, TempZ, TempW};
	}

	TVector4<T> operator*(const TVector4<T>& Other)
	{
		TQuat QuatVector{Other.X, Other.Y, Other.Z, Other.W};
		TQuat Inverse = GetConjugate();
		TQuat Temp = *this;
		TQuat Out = Temp * QuatVector * Inverse;
		return {Out.X, Out.Y, Out.Z, Out.W};
	}

	TQuat GetConjugate() const
	{
		return {-X, -Y, -Z, W};
	}

	TQuat GetInverse() const
	{
		return GetConjugate();
	}

	TVector3<T> RotateVector(TVector3<T> V)
	{
		const TVector3<T> Q(X, Y, Z);
		const TVector3<T> TT = Math::Cross(Q, V) * 2.0f;
		const TVector3<T> Result = V + (TT * W) + Math::Cross(Q, TT);
		return Result;
	}

	T Length()
	{
		return Math::Sqrt(X * X + Y * Y + Z * Z + W * W);
	}

	void Normalize()
	{
		T L = Length();
		if (L == 0)
		{
			X = T(0);
			Y = T(0);
			Z = T(0);
			W = T(1);
		}
		else
		{
			L = T(1) / L;
			X *= L;
			Y *= L;
			Z *= L;
			W *= L;
		}
	}
};

//
// template <>
// rotf quatf::Rotator();
// template <>
// rotd quatd::Rotator();
