#pragma once

#include "Framework/Core/Logging.h"
#include "Math.h"

#include <cassert>
#include <format>

#define EDGE_FUNCTION(X0, Y0, X1, Y1, X2, Y2) (((X1) - (X0)) * ((Y2) - (Y0)) - ((Y1) - (Y0)) * ((X2) - (X0)))

template <typename T>
struct TVector2
{
	static_assert(std::is_arithmetic_v<T>, "Type is not a number.");

	// Memory aligned coordinate values
	union
	{
		struct	  // NOLINT(clang-diagnostic-nested-anon-types)
		{
			T X;
			T Y;
		};
		T XY[2];
	};

	// Constructors
	TVector2() : X(0), Y(0)
	{
#ifdef _DEBUG
		CheckNaN();
#endif
	}
	TVector2(T InX) : X(InX), Y(InX)
	{
#ifdef _DEBUG
		CheckNaN();
#endif
	}
	TVector2(T InX, T InY) : X(InX), Y(InY)
	{
#ifdef _DEBUG
		CheckNaN();
#endif
	}
	TVector2(const std::initializer_list<T>& Values)
	{
		X = *(Values.begin());
		Y = *(Values.begin() + 1);
		{
#ifdef _DEBUG
			CheckNaN();
#endif
		}
	}

	// Functions
	void CheckNaN() const
	{
		if (!(Math::IsFinite(X) && Math::IsFinite(Y)))
		{
			LOG_ERROR("Vector [{}, {}] contains NaN", X, Y)
		}
	}
	static TVector2 ZeroVector()
	{
		return TVector2();
	}
	static TVector2 IdentityVector()
	{
		return TVector2(1);
	}

	template <typename ToType>
	TVector2<ToType> ToType() const
	{
		return {static_cast<ToType>(X), static_cast<ToType>(Y)};
	}

	void Normalize()
	{
		X = 1.0f / X;
		Y = 1.0f / Y;
		{
#ifdef _DEBUG
			CheckNaN();
#endif
		}
	}
	TVector2 Normalized() const
	{
		return {1.0f / X, 1.0f / Y};
	}

	std::string ToString() const
	{
		return std::format("[{}, {}]", X, Y);
	}

	// Operators
	TVector2 operator+(const TVector2& V) const
	{
		return {X + V.X, Y + V.Y};
	}
	TVector2& operator+=(const TVector2& V)
	{
		X += V.X;
		Y += V.Y;
		{
#ifdef _DEBUG
			CheckNaN();
#endif
		}
		return *this;
	}
	TVector2 operator-(const TVector2& V) const
	{
		return {X - V.X, Y - V.Y};
	}
	TVector2& operator-=(const TVector2& V)
	{
		X -= V.X;
		Y -= V.Y;
		{
#ifdef _DEBUG
			CheckNaN();
#endif
		}
		return *this;
	}
	TVector2 operator*(const TVector2& V) const
	{
		return {X * V.X, Y * V.Y};
	}
	TVector2& operator*=(const TVector2& V)
	{
		X *= V.X;
		Y *= V.Y;
		{
#ifdef _DEBUG
			CheckNaN();
#endif
		}
		return *this;
	}
	TVector2 operator/(const TVector2& V) const
	{
		return {X / V.X, Y / V.Y};
	}
	TVector2& operator/=(const TVector2& V)
	{
		X /= V.X;
		Y /= V.Y;
		{
#ifdef _DEBUG
			CheckNaN();
#endif
		}
		return *this;
	}
	TVector2 operator-()
	{
		return TVector2(-X, -Y);
	}
	bool operator<(const TVector2& Other)
	{
		return X < Other.X && Y < Other.Y;
	}
	bool operator>(const TVector2& Other)
	{
		return X > Other.X && Y > Other.Y;
	}
	bool operator>(T Value)
	{
		return X > Value && Y > Value;
	}
	bool operator<(T Value)
	{
		return X < Value && Y < Value;
	}
	bool operator==(T Value)
	{
		return X == Value && Y == Value;
	}
	bool operator==(const TVector2& Other)
	{
		return X == Other.X && Y == Other.Y;
	}
	bool operator!=(const TVector2& Other)
	{
		return X != Other.X || Y != Other.Y;
	}

	T operator[](int32 Index) const
	{
		return XY[Index];
	}
	T& operator[](int32 Index)
	{
		return XY[Index];
	}
};

template <typename T>
struct TVector3
{
	static_assert(std::is_arithmetic_v<T>, "Type is not a number.");

	// Memory aligned coordinate values
	union
	{
		struct	  // NOLINT(clang-diagnostic-nested-anon-types)
		{
			T X;
			T Y;
			T Z;
		};
		T XYZ[3];
	};

	// Constructors
	TVector3() : X(0), Y(0), Z(0)
	{
#ifdef _DEBUG
		CheckNaN();
#endif
	}
	TVector3(T InX) : X(InX), Y(InX), Z(InX)
	{
#ifdef _DEBUG
		CheckNaN();
#endif
	}
	TVector3(T InX, T InY, T InZ) : X(InX), Y(InY), Z(InZ)
	{
#ifdef _DEBUG
		CheckNaN();
#endif
	}
	TVector3(const TVector2<T>& V, T InZ = T(1)) : X(V.X), Y(V.Y), Z(InZ)
	{
#ifdef _DEBUG
		CheckNaN();
#endif
	}
	TVector3(const std::initializer_list<T>& Values)
	{
		X = *(Values.begin());
		Y = *(Values.begin() + 1);
		Z = *(Values.begin() + 2);
#ifdef _DEBUG
		CheckNaN();
#endif
	}

	// Functions
	static TVector3 ZeroVector()
	{
		return TVector3();
	}
	static TVector3 IdentityVector()
	{
		return TVector3(1);
	}
	static TVector3 ForwardVector()
	{
		return TVector3(1, 0, 0);
	}
	static TVector3 BackVector()
	{
		return TVector3(-1, 0, 0);
	}
	static TVector3 UpVector()
	{
		return TVector3(0, 1, 0);
	}
	static TVector3 DownVector()
	{
		return TVector3(0, -1, 0);
	}
	static TVector3 RightVector()
	{
		return TVector3(0, 0, 1);
	}
	static TVector3 LeftVector()
	{
		return TVector3(0, 0, -1);
	}
	void CheckNaN() const
	{
		if (!(Math::IsFinite(X) && Math::IsFinite(Y) && Math::IsFinite(Z)))
		{
			LOG_ERROR("Vector [{}, {}, {}] contains NaN", X, Y, Z)
		}
	}

	template <typename ToType>
	TVector3<ToType> ToType() const
	{
		return TVector3<ToType>{static_cast<ToType>(X), static_cast<ToType>(Y), static_cast<ToType>(Z)};
	}

	void Normalize()
	{
		const T Magnitude = Length();
		if (Magnitude < 0.000001f)
		{
			*this = ZeroVector();
		}
		else
		{
			X /= Magnitude;
			Y /= Magnitude;
			Z /= Magnitude;
		}
		CheckNaN();
	}
	TVector3 Normalized() const
	{
		TVector3 Out(X, Y, Z);
		Out.Normalize();
		return Out;
	}

	constexpr T Length() const
	{
		return Math::Sqrt(X * X + Y * Y + Z * Z);
	}
	TVector3 Cross(const TVector3& V) const
	{
		return TVector3{Y * V.Z - Z * V.Y, X * V.Z - Z * V.X, X * V.Y - Y * V.X};
	}
	T Dot(const TVector3& V) const
	{
		T Result = 0;
		for (int32 Index = 0; Index < 3; Index++)
		{
			Result += XYZ[Index] * V[Index];
		}
		return Result;
	}
	T Size() const
	{
		return Math::Sqrt(Math::Square(X) + Math::Square(Y) + Math::Square(Z));
	}
	TVector3 SwizzleXY() const
	{
		return {Y, X, Z};
	}
	TVector3 SwizzleXZ() const
	{
		return {Z, Y, X};
	}
	TVector3 SwizzleYZ() const
	{
		return {X, Z, Y};
	}
	TVector3 SwizzleXYZ() const
	{
		return {Z, X, Y};
	}

	std::string ToString() const
	{
		return std::format("[{}, {}, {}]", X, Y, Z);
	}

	// Operators
	TVector3 operator+(const TVector3& V) const
	{
		return {X + V.X, Y + V.Y, Z + V.Z};
	}
	TVector3& operator+=(const TVector3& V)
	{
		X += V.X;
		Y += V.Y;
		Z += V.Z;
		{
#ifdef _DEBUG
			CheckNaN();
#endif
		}
		return *this;
	}
	TVector3 operator-(const TVector3& V) const
	{
		return {X - V.X, Y - V.Y, Z - V.Z};
	}
	TVector3& operator-=(const TVector3& V)
	{
		X -= V.X;
		Y -= V.Y;
		Z -= V.Z;
		{
#ifdef _DEBUG
			CheckNaN();
#endif
		}
		return *this;
	}
	TVector3 operator*(const TVector3& V) const
	{
		return {X * V.X, Y * V.Y, Z * V.Z};
	}
	TVector3& operator*=(const TVector3& V)
	{
		X *= V.X;
		Y *= V.Y;
		Z *= V.Z;
		{
#ifdef _DEBUG
			CheckNaN();
#endif
		}
		return *this;
	}
	TVector3 operator/(const TVector3& V) const
	{
		return {X / V.X, Y / V.Y, Z / V.Z};
	}
	TVector3& operator/=(const TVector3& V)
	{
		X /= V.X;
		Y /= V.Y;
		Z /= V.Z;
		{
#ifdef _DEBUG
			CheckNaN();
#endif
		}
		return *this;
	}
	bool operator==(const TVector3& V) const
	{
		return X == V.X && Y == V.Y && Z == V.Z;
	}
	bool operator!=(const TVector3& V) const
	{
		return X != V.X || Y != V.Y || Z != V.Z;
	}

	TVector3 operator-() const
	{
		return TVector3(-X, -Y, -Z);
	}
	bool operator<(const TVector3& Other)
	{
		return X < Other.X && Y < Other.Y && Z < Other.Z;
	}
	bool operator>(const TVector3& Other)
	{
		return X > Other.X && Y > Other.Y && Z > Other.Z;
	}

	T operator[](int32 Index) const
	{
		return XYZ[Index];
	}
	T& operator[](int32 Index)
	{
		return XYZ[Index];
	}

	operator TVector2<T>() const
	{
		return {X, Y};
	}
};

template <typename T>
struct TVector4
{
	static_assert(std::is_arithmetic_v<T>, "Type is not a number.");

	// Memory aligned coordinate values
	union
	{
		struct	  // NOLINT(clang-diagnostic-nested-anon-types)
		{
			T X;
			T Y;
			T Z;
			T W;
		};
		T XYZW[4];
	};

	// Constructors
	TVector4() : X(0), Y(0), Z(0), W(0)
	{
#ifdef _DEBUG
		CheckNaN();
#endif
	}
	TVector4(T InX) : X(InX), Y(InX), Z(InX), W(InX)
	{
#ifdef _DEBUG
		CheckNaN();
#endif
	}
	TVector4(T InX, T InY, T InZ, T InW) : X(InX), Y(InY), Z(InZ), W(InW)
	{
#ifdef _DEBUG
		CheckNaN();
#endif
	}
	TVector4(T* Values) : X(Values[0]), Y(Values[1]), Z(Values[2]), W(Values[3])
	{
	}
	TVector4(const std::initializer_list<T>& Values)
	{
		X = *(Values.begin());
		Y = *(Values.begin() + 1);
		Z = *(Values.begin() + 2);
		W = *(Values.begin() + 3);
#ifdef _DEBUG
		CheckNaN();
#endif
	}
	TVector4(const TVector3<T>& V, T InW = T(1)) : X(V.X), Y(V.Y), Z(V.Z), W(InW)
	{
#ifdef _DEBUG
		CheckNaN();
#endif
	}

	// Functions
	static TVector4 ZeroVector()
	{
		return TVector4();
	}
	static TVector4 IdentityVector()
	{
		return TVector4(1);
	}
	void CheckNaN() const
	{
		if (!(Math::IsFinite(X) && Math::IsFinite(Y) && Math::IsFinite(Z) && Math::IsFinite(W)))
		{
			LOG_ERROR("Vector [{}, {}, {}, {}] contains NaN", X, Y, Z, W)
		}
	}

	template <typename ToType>
	TVector4<ToType> ToType() const
	{
		return {static_cast<ToType>(X), static_cast<ToType>(Y), static_cast<ToType>(Z), static_cast<ToType>(W)};
	}

	void Normalize()
	{
		X = T(1.0) / X;
		Y = T(1.0) / Y;
		Z = T(1.0) / Z;
		W = T(1.0) / W;
	}
	TVector4 Normalized() const
	{
		return {T(1.0) / X, T(1.0) / Y, T(1.0) / Z, T(1.0) / W};
	}

	std::string ToString() const
	{
		return std::format("[{}, {}, {}, {}]", X, Y, Z, W);
	}

	// Operators
	TVector4 operator+(const TVector4& V) const
	{
		return {X + V.X, Y + V.Y, Z + V.Z, W + V.W};
	}
	TVector4& operator+=(const TVector4& V)
	{
		X += V.X;
		Y += V.Y;
		Z += V.Z;
		W += V.W;
#ifdef _DEBUG
		CheckNaN();
#endif
		return *this;
	}
	TVector4 operator-(const TVector4& V) const
	{
		return {X - V.X, Y - V.Y, Z - V.Z, W - V.W};
	}
	TVector4& operator-=(const TVector4& V)
	{
		X -= V.X;
		Y -= V.Y;
		Z -= V.Z;
		W -= V.W;
#ifdef _DEBUG
		CheckNaN();
#endif
		return *this;
	}
	TVector4 operator*(const TVector4& V) const
	{
		return {X * V.X, Y * V.Y, Z * V.Z, W * V.W};
	}
	TVector4& operator*=(const TVector4& V)
	{
		X *= V.X;
		Y *= V.Y;
		Z *= V.Z;
		W *= V.W;
#ifdef _DEBUG
		CheckNaN();
#endif
		return *this;
	}
	TVector4 operator/(const TVector4& V) const
	{
		return {X / V.X, Y / V.Y, Z / V.Z, W / V.W};
	}
	TVector4& operator/=(const TVector4& V)
	{
		X /= V.X;
		Y /= V.Y;
		Z /= V.Z;
		W /= V.W;
#ifdef _DEBUG
		CheckNaN();
#endif
		return *this;
	}
	TVector4 operator-()
	{
		return TVector4(-X, -Y, -Z, -W);
	}
	bool operator<(const TVector4& Other)
	{
		return X < Other.X && Y < Other.Y && Z < Other.Z;
	}
	bool operator>(const TVector4& Other)
	{
		return X > Other.X && Y > Other.Y && Z > Other.Z && W > Other.W;
	}

	T operator[](int32 Index) const
	{
		return XYZW[Index];
	}
	T& operator[](int32 Index)
	{
		return XYZW[Index];
	}

	operator TVector2<T>() const
	{
		return {X, Y};
	}

	operator TVector3<T>() const
	{
		return {X, Y, Z};
	}
};

namespace Math
{
template <typename T>
static T Cross(const TVector2<T>& A, const TVector2<T>& B)
{
	return A.X * B.Y - A.Y * B.X;
}

/**
 * @brief Computes the cross product of two 3D vectors.
 * @tparam T The type of the vector elements.
 * @param A The first vector.
 * @param B The second vector.
 * @return The cross product of A and B.
 */
template <typename T>
static TVector3<T> Cross(const TVector3<T>& A, const TVector3<T>& B)
{
	return TVector3<T>{A.Y * B.Z - B.Y * A.Z, A.Z * B.X - B.Z * A.X, A.X * B.Y - B.X * A.Y};
}

template <typename T>
static T Dot(const TVector3<T>& A, const TVector3<T>& B)
{
	TVector3<T> Tmp = A * B;
	return Tmp.X + Tmp.Y + Tmp.Z;
}

template <typename T>
static T CrossDot(const TVector3<T>& A, const TVector3<T>& B, const TVector3<T>& P)
{
	return Math::Dot(Math::Cross(A, B), P);
}

/* Distance between two points in 3D space */
template <typename T>
static T Distance(const TVector3<T> V0, const TVector3<T>& V1)
{
	T A = Math::Square(V1.X - V0.X);
	T B = Math::Square(V1.Y - V0.Y);
	T C = Math::Square(V1.Z - V0.Z);
	return Math::Sqrt(A + B + C);
}

template <typename T>
static T Dot(const TVector4<T>& A, const TVector4<T>& B)
{
	T Result = T(0);
	for (int32 Index = 0; Index < 4; Index++)
	{
		Result += A[Index] * B[Index];
	}
	return Result;
}

template <typename T>
static T Area2D(const TVector3<T>& V0, const TVector3<T>& V1, const TVector3<T>& V2)
{
	T A = V0.X * (V1.Y - V2.Y);
	T B = V1.X * (V2.Y - V0.Y);
	T C = V2.X * (V0.Y - V1.Y);
	return Math::Abs((A + B + C) / T(2));
}

// Vector Sign
template <typename T>
static bool EdgeSign(const TVector2<T>& A, const TVector2<T>& B, const TVector2<T>& C)
{
	T Result = (C[0] - A[0]) * (B[1] - A[1]) - (C[1] - A[1]) * (B[0] - A[0]);
	return Math::Sign(Result);
}

template <typename T>
static float EdgeValue(const TVector2<T>& A, const TVector2<T>& B, const TVector2<T>& P)
{
	return Math::Cross(B - A, P - A);
}

/**
 * Calculates the barycentric coordinates of a point P with respect to a triangle defined by vertices V0, V1, and V2.
 *
 * @param P The point to calculate the barycentric coordinates for.
 * @param V0 The first vertex of the triangle.
 * @param V1 The second vertex of the triangle.
 * @param V2 The third vertex of the triangle.
 * @param UVW The output parameter that will store the calculated barycentric coordinates.
 * @param Tolerance The tolerance value for determining if the point is inside the triangle.
 * @return True if the point is inside the triangle, false otherwise.
 */
template <typename T>
static bool GetBarycentric(const TVector3<T>& P, const TVector3<T>& V0, const TVector3<T>& V1, const TVector3<T>& V2,
	TVector3<T>& UVW, T Tolerance = P_VERY_SMALL_NUMBER)
{
	// Calculate the vectors representing the edges of the triangle
	const TVector3<T> BA = V1 - V0;
	const TVector3<T> CA = V2 - V0;
	const TVector3<T> PA = P - V0;

	// Calculate the dot products
	const T D00 = BA.Dot(BA);
	const T D01 = BA.Dot(CA);
	const T D11 = CA.Dot(CA);
	const T D20 = PA.Dot(BA);
	const T D21 = PA.Dot(CA);

	// Calculate the denominator of the formula
	const T Denom = T(1) / (D00 * D11 - D01 * D01);

	// Calculate the barycentric coordinates
	const T V = (D11 * D20 - D01 * D21) * Denom;
	const T W = (D00 * D21 - D01 * D20) * Denom;
	const T U = T(1) - V - W;

	// Store the barycentric coordinates in the output parameter
	UVW.X = U;
	UVW.Y = V;
	UVW.Z = W;

	// Check if the point is inside the triangle
	return (UVW.X >= T(0) && UVW.Y >= T(0) && UVW.Z >= T(0));
}

template <typename T>
static EWindingOrder GetVertexOrder(const TVector3<T>& V0, const TVector3<T>& V1, const TVector3<T>& V2)
{
	const float Result = (V1.Y - V0.Y) * (V2.X - V1.X) - (V1.X - V0.X) * (V2.Y - V1.Y);
	if (Result == T(0))
	{
		return EWindingOrder::CL;
	}
	return Result > T(0) ? EWindingOrder::CW : EWindingOrder::CCW;
}

template <typename T>
static void GetLine(const TVector3<T>& A, const TVector3<T>& B, std::vector<IVector2>& Points, int32 Width, int32 Height)
{
	int32 X0 = static_cast<int32>(A.X);
	int32 Y0 = static_cast<int32>(A.Y);
	int32 X1 = static_cast<int32>(B.X);
	int32 Y1 = static_cast<int32>(B.Y);
	int32 DeltaX = Abs(X1 - X0);
	int32 DeltaY = Abs(Y1 - Y0);
	int32 StepX = (X0 < X1) ? 1 : -1;
	int32 StepY = (Y0 < Y1) ? 1 : -1;
	int32 Error = DeltaX - DeltaY;

	while (true)
	{
		if (X0 >= 0 && X0 < Width && Y0 >= 0 && Y0 < Height)
		{
			Points.emplace_back(X0, Y0);
		}

		if (X0 == X1 && Y0 == Y1)
		{
			break;
		}

		const int32 DoubleError = Error * 2;
		if (DoubleError > -DeltaY)
		{
			Error -= DeltaY;
			X0 += StepX;
		}
		if (DoubleError < DeltaX)
		{
			Error += DeltaX;
			Y0 += StepY;
		}
	}
}

static constexpr float EdgeFunction(float X0, float Y0, float X1, float Y1, float X2, float Y2)
{
	return (X1 - X0) * (Y2 - Y0) - (Y1 - Y0) * (X2 - X0);
}

template <typename T>
static T EdgeFunction(const TVector3<T>& A, const TVector3<T>& B, const TVector3<T>& C)
{
	return (B.X - A.X) * (C.Y - A.Y) - (B.Y - A.Y) * (C.X - A.X);
}

template <typename T>
static T GetDepth(const TVector3<T>& P, const TVector3<T>& V0, const TVector3<T>& V1, const TVector3<T>& V2, T Area)
{
	// Calculate depth
	T W0 = Math::EdgeFunction(V1, V2, P);
	T W1 = Math::EdgeFunction(V2, V0, P);
	T W2 = Math::EdgeFunction(V0, V1, P);

	W0 /= Area;
	W1 /= Area;
	W2 /= Area;

	return V0.Z * W0 + V1.Z * W1 + V2.Z * W2;
}

// // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
template <typename T>
static TVector3<T> GetSurfaceNormal(const TVector3<T>& V0, const TVector3<T>& V1, const TVector3<T>& V2)
{
	TVector3<T> Edge0 = V0 - V2;
	TVector3<T> Edge1 = V1 - V2;
	TVector3<T> Normal = Math::Cross(Edge0, Edge1);
	return Normal.Normalized();
}
};	  // namespace Math
