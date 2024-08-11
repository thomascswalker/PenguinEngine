#pragma once

#include <string>

#include "Math.h"
#include "Plane.h"
#include "Rotator.h"
#include "Vector.h"
#include "Framework/Core/Logging.h"

// m[RowIndex][ColumnIndex]
template <typename T>
struct mat4_t
{
	static_assert(std::is_floating_point_v<T>, "Type is not floating point.");

	alignas(16) T m[4][4];

	mat4_t()
	{
		setIdentity();
#if _DEBUG
		checkNaN();
#endif
	}

	mat4_t(const plane_t<T>& inX, const plane_t<T>& inY, const plane_t<T>& inZ, const plane_t<T>& inW)
	{
		m[0][0] = inX.x;
		m[0][1] = inX.y;
		m[0][2] = inX.z;
		m[0][3] = inX.w;
		m[1][0] = inY.x;
		m[1][1] = inY.y;
		m[1][2] = inY.z;
		m[1][3] = inY.w;
		m[2][0] = inZ.x;
		m[2][1] = inZ.y;
		m[2][2] = inZ.z;
		m[2][3] = inZ.w;
		m[3][0] = inW.x;
		m[3][1] = inW.y;
		m[3][2] = inW.z;
		m[3][3] = inW.w;
#if _DEBUG
		checkNaN();
#endif
	}

	mat4_t(const vec3_t<T>& inX, const vec3_t<T>& inY, const vec3_t<T>& inZ, const vec3_t<T>& inW)
	{
		m[0][0] = inX.x;
		m[0][1] = inX.y;
		m[0][2] = inX.z;
		m[0][3] = 0.0f;
		m[1][0] = inY.x;
		m[1][1] = inY.y;
		m[1][2] = inY.z;
		m[1][3] = 0.0f;
		m[2][0] = inZ.x;
		m[2][1] = inZ.y;
		m[2][2] = inZ.z;
		m[2][3] = 0.0f;
		m[3][0] = inW.x;
		m[3][1] = inW.y;
		m[3][2] = inW.z;
		m[3][3] = 1.0f;
#if _DEBUG
		checkNaN();
#endif
	}

	mat4_t(const vec4_t<T>& inX, const vec4_t<T>& inY, const vec4_t<T>& inZ, const vec4_t<T>& inW)
	{
		m[0][0] = inX.x;
		m[0][1] = inX.y;
		m[0][2] = inX.z;
		m[0][3] = inX.w;
		m[1][0] = inY.x;
		m[1][1] = inY.y;
		m[1][2] = inY.z;
		m[1][3] = inY.w;
		m[2][0] = inZ.x;
		m[2][1] = inZ.y;
		m[2][2] = inZ.z;
		m[2][3] = inZ.w;
		m[3][0] = inW.x;
		m[3][1] = inW.y;
		m[3][2] = inW.z;
		m[3][3] = inW.w;
#if _DEBUG
		checkNaN();
#endif
	}

	mat4_t(T m0, T m1, T m2, T m3, T m4, T m5, T m6, T m7, T m8, T m9, T m10, T m11, T m12, T m13, T m14, T m15)
	{
		m[0][0] = m0;
		m[0][1] = m1;
		m[0][2] = m2;
		m[0][3] = m3;
		m[1][0] = m4;
		m[1][1] = m5;
		m[1][2] = m6;
		m[1][3] = m7;
		m[2][0] = m8;
		m[2][1] = m9;
		m[2][2] = m10;
		m[2][3] = m11;
		m[3][0] = m12;
		m[3][1] = m13;
		m[3][2] = m14;
		m[3][3] = m15;
#if _DEBUG
		checkNaN();
#endif
	}

	mat4_t(const mat4_t& other)
	{
		std::memcpy(m, &other.m, 16 * sizeof(T));
#if _DEBUG
		checkNaN();
#endif
	}

	mat4_t(mat4_t&& other) noexcept
	{
		std::memcpy(m, &other.m, 16 * sizeof(T));
#if _DEBUG
		checkNaN();
#endif
	}

	void checkNaN() const
	{
		for (int32 x = 0; x < 4; x++)
		{
			for (int32 y = 0; y < 4; y++)
			{
				if (!Math::isFinite(m[x][y]))
				{
					LOG_ERROR("Matrix cell [{},{}] contains NaN", x, y)
				}
			}
		}
	}

	mat4_t flip() const
	{
		mat4_t out;

		out.m[0][0] = m[0][0];
		out.m[0][1] = m[1][0];
		out.m[0][2] = m[2][0];
		out.m[0][3] = m[3][0];
		out.m[1][0] = m[0][1];
		out.m[1][1] = m[1][1];
		out.m[1][2] = m[2][1];
		out.m[1][3] = m[3][1];
		out.m[2][0] = m[0][2];
		out.m[2][1] = m[1][2];
		out.m[2][2] = m[2][2];
		out.m[2][3] = m[3][2];
		out.m[3][0] = m[0][3];
		out.m[3][1] = m[1][3];
		out.m[3][2] = m[2][3];
		out.m[3][3] = m[3][3];

		return out;
	}

	// Functions

	bool equals(const mat4_t& other, T threshold = 0.00001f) const
	{
		for (int32 x = 0; x < 4; x++)
		{
			for (int32 y = 0; y < 4; y++)
			{
				if (m[x][y] - other.m[x][y] > threshold)
				{
					return false;
				}
			}
		}
		return true;
	}

	static mat4_t getIdentity()
	{
		return mat4_t{
			plane_t<T>(1, 0, 0, 0),
			plane_t<T>(0, 1, 0, 0),
			plane_t<T>(0, 0, 1, 0),
			plane_t<T>(0, 0, 0, 1)
		};
	}

	void setIdentity()
	{
		m[0][0] = 1;
		m[0][1] = 0;
		m[0][2] = 0;
		m[0][3] = 0;
		m[1][0] = 0;
		m[1][1] = 1;
		m[1][2] = 0;
		m[1][3] = 0;
		m[2][0] = 0;
		m[2][1] = 0;
		m[2][2] = 1;
		m[2][3] = 0;
		m[3][0] = 0;
		m[3][1] = 0;
		m[3][2] = 0;
		m[3][3] = 1;
	}

	void setZero()
	{
		m[0][0] = 0;
		m[0][1] = 0;
		m[0][2] = 0;
		m[0][3] = 0;
		m[1][0] = 0;
		m[1][1] = 0;
		m[1][2] = 0;
		m[1][3] = 0;
		m[2][0] = 0;
		m[2][1] = 0;
		m[2][2] = 0;
		m[2][3] = 0;
		m[3][0] = 0;
		m[3][1] = 0;
		m[3][2] = 0;
		m[3][3] = 0;
	}

	[[nodiscard]] constexpr T getDeterminant() const
	{
		T a11 = m[0][0];
		T a12 = m[0][1];
		T a13 = m[0][2];
		T a14 = m[0][3];
		T a21 = m[1][0];
		T a22 = m[1][1];
		T a23 = m[1][2];
		T a24 = m[1][3];
		T a31 = m[2][0];
		T a32 = m[2][1];
		T a33 = m[2][2];
		T a34 = m[2][3];
		T a41 = m[3][0];
		T a42 = m[3][1];
		T a43 = m[3][2];
		T a44 = m[3][3];

		T det1 = a11 * a22 * a33 * a44;
		T det2 = a11 * a23 * a34 * a42;
		T det3 = a11 * a24 * a32 * a43;

		T det4 = a12 * a21 * a34 * a43;
		T det5 = a12 * a23 * a31 * a44;
		T det6 = a12 * a24 * a33 * a41;

		T det7 = a13 * a21 * a32 * a44;
		T det8 = a13 * a22 * a34 * a41;
		T det9 = a13 * a24 * a31 * a42;

		T det10 = a14 * a21 * a33 * a42;
		T det11 = a14 * a22 * a31 * a43;
		T det12 = a14 * a23 * a32 * a41;

		T det13 = a11 * a22 * a34 * a43;
		T det14 = a11 * a23 * a32 * a44;
		T det15 = a11 * a24 * a33 * a42;

		T det16 = a12 * a21 * a33 * a44;
		T det17 = a12 * a23 * a34 * a41;
		T det18 = a12 * a24 * a31 * a43;

		T det19 = a13 * a21 * a34 * a42;
		T det20 = a13 * a22 * a31 * a44;
		T det21 = a13 * a24 * a32 * a41;

		T det22 = a14 * a21 * a32 * a43;
		T det23 = a14 * a22 * a33 * a41;
		T det24 = a14 * a23 * a31 * a42;

		return (
			det1 + det2 + det3 +
			det4 + det5 + det6 +
			det7 + det8 + det9 +
			det10 + det11 + det12 -
			det13 - det14 - det15 -
			det16 - det17 - det18 -
			det19 - det20 - det21 -
			det22 - det23 - det24
		);
	}

	// http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche0023.html
	mat4_t getInverse()
	{
		T determinant = getDeterminant();
		if (determinant == 0.0 || !Math::isFinite(determinant))
		{
			return getIdentity();
		}
		determinant = 1.0f / determinant;

		T a2323 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
		T a1323 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
		T a1223 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
		T a0323 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
		T a0223 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
		T a0123 = m[2][0] * m[3][1] - m[2][1] * m[3][0];
		T a2313 = m[1][2] * m[3][3] - m[1][3] * m[3][2];
		T a1313 = m[1][1] * m[3][3] - m[1][3] * m[3][1];
		T a1213 = m[1][1] * m[3][2] - m[1][2] * m[3][1];
		T a2312 = m[1][2] * m[2][3] - m[1][3] * m[2][2];
		T a1312 = m[1][1] * m[2][3] - m[1][3] * m[2][1];
		T a1212 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
		T a0313 = m[1][0] * m[3][3] - m[1][3] * m[3][0];
		T a0213 = m[1][0] * m[3][2] - m[1][2] * m[3][0];
		T a0312 = m[1][0] * m[2][3] - m[1][3] * m[2][0];
		T a0212 = m[1][0] * m[2][2] - m[1][2] * m[2][0];
		T a0113 = m[1][0] * m[3][1] - m[1][1] * m[3][0];
		T a0112 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

		T result[4][4]{};

		result[0][0] = determinant * (m[1][1] * a2323 - m[1][2] * a1323 + m[1][3] * a1223);
		result[0][1] = determinant * -(m[0][1] * a2323 - m[0][2] * a1323 + m[0][3] * a1223);
		result[0][2] = determinant * (m[0][1] * a2313 - m[0][2] * a1313 + m[0][3] * a1213);
		result[0][3] = determinant * -(m[0][1] * a2312 - m[0][2] * a1312 + m[0][3] * a1212);
		result[1][0] = determinant * -(m[1][0] * a2323 - m[1][2] * a0323 + m[1][3] * a0223);
		result[1][1] = determinant * (m[0][0] * a2323 - m[0][2] * a0323 + m[0][3] * a0223);
		result[1][2] = determinant * -(m[0][0] * a2313 - m[0][2] * a0313 + m[0][3] * a0213);
		result[1][3] = determinant * (m[0][0] * a2312 - m[0][2] * a0312 + m[0][3] * a0212);
		result[2][0] = determinant * (m[1][0] * a1323 - m[1][1] * a0323 + m[1][3] * a0123);
		result[2][1] = determinant * -(m[0][0] * a1323 - m[0][1] * a0323 + m[0][3] * a0123);
		result[2][2] = determinant * (m[0][0] * a1313 - m[0][1] * a0313 + m[0][3] * a0113);
		result[2][3] = determinant * -(m[0][0] * a1312 - m[0][1] * a0312 + m[0][3] * a0112);
		result[3][0] = determinant * -(m[1][0] * a1223 - m[1][1] * a0223 + m[1][2] * a0123);
		result[3][1] = determinant * (m[0][0] * a1223 - m[0][1] * a0223 + m[0][2] * a0123);
		result[3][2] = determinant * -(m[0][0] * a1213 - m[0][1] * a0213 + m[0][2] * a0113);
		result[3][3] = determinant * (m[0][0] * a1212 - m[0][1] * a0212 + m[0][2] * a0112);

		mat4_t out;
		std::memcpy(out.m, &result, 16 * sizeof(T));
#if _DEBUG
		checkNaN();
#endif
		return out;
	}

	vec3_t<T> getAxis(const EAxis inAxis) const
	{
		switch (inAxis)
		{
		case EAxis::X:
			{
				return vec3_t(m[0][0], m[1][0], m[2][0]);
			}
		case EAxis::Y:
			{
				return vec3_t(m[0][1], m[1][1], m[2][1]);
			}
		case EAxis::Z:
			{
				return vec3_t(m[0][2], m[1][2], m[2][2]);
			}
		}
		return vec3_t<T>::zeroVector();
	}

	vec3_t<T> getAxisNormalized(const EAxis inAxis) const
	{
		return getAxis(inAxis).normalized();
	}

	rot_t<T> getRotator() const
	{
		const vec3_t XAxis = getAxis(EAxis::X);
		const vec3_t YAxis = getAxis(EAxis::Y);
		const vec3_t ZAxis = getAxis(EAxis::Z);
		const T RadToDeg = 180.0f / (PI * 2.0f);

		T Pitch = std::atan2f(XAxis.z, std::sqrtf(Math::square(XAxis.x) + Math::square(XAxis.y))) * RadToDeg;
		T Yaw = std::atan2f(XAxis.y, XAxis.x) * RadToDeg;
		rot_t Rotator = rot_t(Pitch, Yaw, T(0));

		const vec3_t SYAxis = mat4_rot_t<T>(Rotator).getAxis(EAxis::Y);
		Rotator.Roll = std::atan2f(Math::dot(ZAxis, SYAxis), Math::dot(YAxis, SYAxis)) * RadToDeg;

		return Rotator;
	}

	vec3_t<T> getScale(T tolerance = 0.00000001f);

	vec3_t<T> getTranslation()
	{
		return vec3_t(m[3][0], m[3][1], m[3][2]);
	}

	static mat4_t makeFromX(T Angle)
	{
		T C = std::cosf(Angle);
		T S = std::sinf(Angle);
		return mat4_t(
			plane_t<T>(1, 0, 0, 0),
			plane_t<T>(0, C, -S, 0),
			plane_t<T>(0, S, C, 0),
			plane_t<T>(0, 0, 0, 1)
		);
	}

	static mat4_t makeFromY(T Angle)
	{
		T C = std::cosf(Angle);
		T S = std::sinf(Angle);
		return mat4_t(
			plane_t<T>(C, 0, S, 0),
			plane_t<T>(0, 1, 0, 0),
			plane_t<T>(-S, 0, C, 0),
			plane_t<T>(0, 0, 0, 1)
		);
	}

	static mat4_t makeFromZ(T Angle)
	{
		T C = std::cosf(Angle);
		T S = std::sinf(Angle);
		return mat4_t(
			plane_t<T>(C, -S, 0, 0),
			plane_t<T>(S, C, 0, 0),
			plane_t<T>(0, 0, 1, 0),
			plane_t<T>(0, 0, 0, 1)
		);
	}

	T get(int32 X, int32 Y) const
	{
		return m[X][Y];
	}

	vec4_t<T> getRow(int32 row) const;

	vec4_t<T> getColumn(int32 Column) const
	{
		return {m[0][Column], m[1][Column], m[2][Column], m[3][Column]};
	}

	void set(int32 X, int32 Y, T Value)
	{
		m[X][Y] = Value;
	}

	std::string toString() const
	{
		std::string Output;

		Output += std::format("[{} {} {} {}]\n", m[0][0], m[0][1], m[0][2], m[0][3]);
		Output += std::format("[{} {} {} {}]\n", m[1][0], m[1][1], m[1][2], m[1][3]);
		Output += std::format("[{} {} {} {}]\n", m[2][0], m[2][1], m[2][2], m[2][3]);
		Output += std::format("[{} {} {} {}]\n", m[3][0], m[3][1], m[3][2], m[3][3]);

		return Output;
	}

	mat4_t operator+(const mat4_t& Other) const
	{
		mat4_t Result;
		for (int32 X = 0; X < 4; X++)
		{
			for (int32 Y = 0; Y < 4; Y++)
			{
				Result.m[X][Y] = m[X][Y] + Other.m[X][Y];
			}
		}
		return Result;
	}

	mat4_t& operator+=(const mat4_t& Other)
	{
		*this = *this + Other;
#if _DEBUG
		checkNaN();
#endif
		return *this;
	}

	mat4_t operator-(const mat4_t& Other) const
	{
		mat4_t Result;
		for (int32 X = 0; X < 4; X++)
		{
			for (int32 Y = 0; Y < 4; Y++)
			{
				Result.m[X][Y] = m[X][Y] - Other.m[X][Y];
			}
		}
		return Result;
	}

	mat4_t& operator-=(const mat4_t& other)
	{
		*this = *this - other;
#if _DEBUG
		checkNaN();
#endif
		return *this;
	}

	mat4_t& operator*=(const mat4_t& other)
	{
		*this = *this * other;
#if _DEBUG
		checkNaN();
#endif
		return *this;
	}

	mat4_t operator/(const mat4_t& other) const
	{
		mat4_t result;
		for (int32 x = 0; x < 4; x++)
		{
			for (int32 y = 0; y < 4; y++)
			{
				result.m[x][y] = m[x][y] / other.m[x][y];
			}
		}
		return result;
	}

	mat4_t& operator/=(const mat4_t& other)
	{
		*this = *this / other;
#if _DEBUG
		checkNaN();
#endif
		return *this;
	}

	vec3_t<T> operator*(const vec3_t<T>& v) const
	{
		vec3_t<T> result;
		for (int32 index = 0; index < 3; index++)
		{
			vec4_t<T> rowVector({m[index][0], m[index][1], m[index][2], m[index][3]});
			result[index] = Math::dot(rowVector, vec4_t<T>(v));
		}
		return result;
	}

	vec4_t<T> operator*(const vec4_t<T>& v) const
	{
		T tempX = v.x * m[0][0] + v.y * m[0][1] + v.z * m[0][2] + v.w * m[0][3];
		T tempY = v.x * m[1][0] + v.y * m[1][1] + v.z * m[1][2] + v.w * m[1][3];
		T tempZ = v.x * m[2][0] + v.y * m[2][1] + v.z * m[2][2] + v.w * m[2][3];
		T tempW = v.x * m[3][0] + v.y * m[3][1] + v.z * m[3][2] + v.w * m[3][3];

		return {tempX, tempY, tempZ, tempW};
	}

	mat4_t& operator=(const mat4_t& Other) // NOLINT
	{
		std::memcpy(m, &Other.m, 16 * sizeof(T));
#if _DEBUG
		checkNaN();
#endif
		return *this;
	}

	bool equals(const mat4_t& other)
	{
		for (int32 x = 0; x < 4; x++)
		{
			for (int32 y = 0; y < 4; y++)
			{
				if (m[x][y] != other.m[x][y])
				{
					return false;
				}
			}
		}
		return true;
	}
};

template <typename T>
constexpr mat4_t<T> operator*(const mat4_t<T>& M0, const mat4_t<T>& M1)
{
	using Float4x4 = float[4][4];
	const Float4x4& A = *((const Float4x4*)M0.m);
	const Float4x4& B = *((const Float4x4*)M1.m);
	Float4x4 Temp;
	Temp[0][0] = A[0][0] * B[0][0] + A[0][1] * B[1][0] + A[0][2] * B[2][0] + A[0][3] * B[3][0];
	Temp[0][1] = A[0][0] * B[0][1] + A[0][1] * B[1][1] + A[0][2] * B[2][1] + A[0][3] * B[3][1];
	Temp[0][2] = A[0][0] * B[0][2] + A[0][1] * B[1][2] + A[0][2] * B[2][2] + A[0][3] * B[3][2];
	Temp[0][3] = A[0][0] * B[0][3] + A[0][1] * B[1][3] + A[0][2] * B[2][3] + A[0][3] * B[3][3];

	Temp[1][0] = A[1][0] * B[0][0] + A[1][1] * B[1][0] + A[1][2] * B[2][0] + A[1][3] * B[3][0];
	Temp[1][1] = A[1][0] * B[0][1] + A[1][1] * B[1][1] + A[1][2] * B[2][1] + A[1][3] * B[3][1];
	Temp[1][2] = A[1][0] * B[0][2] + A[1][1] * B[1][2] + A[1][2] * B[2][2] + A[1][3] * B[3][2];
	Temp[1][3] = A[1][0] * B[0][3] + A[1][1] * B[1][3] + A[1][2] * B[2][3] + A[1][3] * B[3][3];

	Temp[2][0] = A[2][0] * B[0][0] + A[2][1] * B[1][0] + A[2][2] * B[2][0] + A[2][3] * B[3][0];
	Temp[2][1] = A[2][0] * B[0][1] + A[2][1] * B[1][1] + A[2][2] * B[2][1] + A[2][3] * B[3][1];
	Temp[2][2] = A[2][0] * B[0][2] + A[2][1] * B[1][2] + A[2][2] * B[2][2] + A[2][3] * B[3][2];
	Temp[2][3] = A[2][0] * B[0][3] + A[2][1] * B[1][3] + A[2][2] * B[2][3] + A[2][3] * B[3][3];

	Temp[3][0] = A[3][0] * B[0][0] + A[3][1] * B[1][0] + A[3][2] * B[2][0] + A[3][3] * B[3][0];
	Temp[3][1] = A[3][0] * B[0][1] + A[3][1] * B[1][1] + A[3][2] * B[2][1] + A[3][3] * B[3][1];
	Temp[3][2] = A[3][0] * B[0][2] + A[3][1] * B[1][2] + A[3][2] * B[2][2] + A[3][3] * B[3][2];
	Temp[3][3] = A[3][0] * B[0][3] + A[3][1] * B[1][3] + A[3][2] * B[2][3] + A[3][3] * B[3][3];

	mat4_t<T> Result;
	memcpy(&Result.m, &Temp, 16 * sizeof(float));
	return Result;
}

template <typename T>
struct mat4_persp_t : mat4_t<T>
{
	mat4_persp_t(T Fov, T Aspect, T MinZ, T MaxZ = MAX_Z) : mat4_t<T>()
	{
		this->setZero();
		const T TanHalfFov = std::tanf(Fov / 2.0f);
		this->m[0][0] = T(1) / (Aspect * TanHalfFov);
		this->m[1][1] = T(1) / TanHalfFov;

		this->m[2][2] = -(MaxZ + MinZ) / (MaxZ - MinZ);
		this->m[3][2] = -T(1);
		this->m[2][3] = -(T(2) * MaxZ * MinZ) / (MaxZ - MinZ);
#if _DEBUG
		this->checkNaN();
#endif
	}
};

template <typename T>
struct mat4_lookat_t : mat4_t<T>
{
	mat4_lookat_t(const vec3_t<T>& Eye, const vec3_t<T>& Center, const vec3_t<T>& UpVector) : mat4_t<T>()
	{
		// Forward vector
		const vec3_t<T> Forward = (Center - Eye).normalized();

		// Right vector
		const vec3_t<T> Right = (Math::cross(Forward, UpVector)).normalized();

		// Up vector
		const vec3_t<T> Up = Math::cross(Right, Forward);

		//  Rx |  Ux | -Fx | -Tx
		//  Ry |  Uy | -Fy | -Ty
		//  Rz |  Uz | -Fz | -Tz 
		//  0  |  0  |  0  |  1
		this->setIdentity();
		this->m[0][0] = Right.x;
		this->m[0][1] = Right.y;
		this->m[0][2] = Right.z;
		this->m[1][0] = Up.x;
		this->m[1][1] = Up.y;
		this->m[1][2] = Up.z;
		this->m[2][0] = -Forward.x;
		this->m[2][1] = -Forward.y;
		this->m[2][2] = -Forward.z;
		this->m[0][3] = -Math::dot(Right, Eye);
		this->m[1][3] = -Math::dot(Up, Eye);
		this->m[2][3] = Math::dot(Forward, Eye);

#if _DEBUG
		this->checkNaN();
#endif
	}
};

template <typename T>
struct mat4_trans_t : mat4_t<T>
{
	mat4_trans_t(const vec3_t<T>& Delta) : mat4_t<T>()
	{
		this->m[3][0] = Delta.x;
		this->m[3][1] = Delta.y;
		this->m[3][2] = Delta.z;

#if _DEBUG
		this->checkNaN();
#endif
	}
};

template <typename T>
struct mat4_rot_t : mat4_t<T>
{
	mat4_rot_t(T Pitch, T Yaw, T Roll) : mat4_t<T>()
	{
		// Convert from degrees to radians
		Pitch = Math::degreesToRadians(Pitch);
		Yaw = Math::degreesToRadians(Yaw);
		Roll = Math::degreesToRadians(Roll);

		T CP = std::cosf(Pitch);
		T SP = std::sinf(Pitch);
		T CY = std::cosf(Yaw);
		T SY = std::sinf(Yaw);
		T CR = std::cosf(Roll);
		T SR = std::sinf(Roll);

		T CPSY = CP * SY;
		T SPSY = SP * SY;

		this->m[0][0] = CY * CR;
		this->m[0][1] = -CY * SR;
		this->m[0][2] = SY;
		this->m[1][0] = SPSY * CR + CP * SR;
		this->m[1][1] = -SPSY * SR + CP * CR;
		this->m[1][2] = -SP * CY;
		this->m[2][0] = -CPSY * CR + SP * SR;
		this->m[2][1] = CPSY * SR + SP * CR;
		this->m[2][2] = CP * CY;

#if _DEBUG
		this->checkNaN();
#endif
	}

	mat4_rot_t(const rot_t<T>& Rotation) : mat4_t<T>()
	{
		*this = mat4_rot_t(Rotation.Pitch, Rotation.Yaw, Rotation.Roll);

#if _DEBUG
		this->checkNaN();
#endif
	}
};

template <typename T>
struct mat4_rottrans_t : mat4_t<T>
{
	mat4_rottrans_t(const rot_t<T>& Rotation, const vec3_t<T>& Translation) : mat4_t<T>()
	{
		mat4_t<T> RotationMatrix = mat4_rot_t<T>(Rotation);
		mat4_t<T> TranslationMatrix = mat4_trans_t<T>(Translation);
		*this = RotationMatrix * TranslationMatrix;

#if _DEBUG
		this->checkNaN();
#endif
	}
};
