#pragma once

#include <string>

#include "Math.h"
#include "Plane.h"
#include "Rotator.h"
#include "Vector.h"
#include "Core/Logging.h"

// m[RowIndex][ColumnIndex]
template <typename T>
struct mat4_t
{
	static_assert(std::is_floating_point_v<T>, "Type is not floating point.");

	alignas(16) T m[4][4];

	mat4_t()
	{
		setIdentity();
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
	}

	mat4_t(const mat4_t& other)
	{
		std::memcpy(m, &other.m, 16 * sizeof(T));
	}

	mat4_t(mat4_t&& other) noexcept
	{
		std::memcpy(m, &other.m, 16 * sizeof(T));
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
		const vec3_t xAxis = getAxis(EAxis::X);
		const vec3_t yAxis = getAxis(EAxis::Y);
		const vec3_t zAxis = getAxis(EAxis::Z);
		const T radToDeg   = 180.0f / (PI * 2.0f);

		T pitch       = std::atan2f(xAxis.z, std::sqrtf(Math::square(xAxis.x) + Math::square(xAxis.y))) * radToDeg;
		T yaw         = std::atan2f(xAxis.y, xAxis.x) * radToDeg;
		rot_t rotator = rot_t(pitch, yaw, T(0));

		const vec3_t syAxis = mat4_rot_t<T>(rotator).getAxis(EAxis::Y);
		rotator.roll        = std::atan2f(zAxis.dot(syAxis), yAxis.dot(syAxis)) * radToDeg;

		return rotator;
	}

	vec3_t<T> getScale(T tolerance = 0.00000001f);

	vec3_t<T> getTranslation()
	{
		return vec3_t(m[3][0], m[3][1], m[3][2]);
	}

	static mat4_t makeFromX(T angle)
	{
		T c = std::cosf(angle);
		T s = std::sinf(angle);
		return mat4_t(
			plane_t<T>(1, 0, 0, 0),
			plane_t<T>(0, c, -s, 0),
			plane_t<T>(0, s, c, 0),
			plane_t<T>(0, 0, 0, 1)
		);
	}

	static mat4_t makeFromY(T angle)
	{
		T c = std::cosf(angle);
		T s = std::sinf(angle);
		return mat4_t(
			plane_t<T>(c, 0, s, 0),
			plane_t<T>(0, 1, 0, 0),
			plane_t<T>(-s, 0, c, 0),
			plane_t<T>(0, 0, 0, 1)
		);
	}

	static mat4_t makeFromZ(T angle)
	{
		T c = std::cosf(angle);
		T s = std::sinf(angle);
		return mat4_t(
			plane_t<T>(c, -s, 0, 0),
			plane_t<T>(s, c, 0, 0),
			plane_t<T>(0, 0, 1, 0),
			plane_t<T>(0, 0, 0, 1)
		);
	}

	T get(int32 x, int32 y) const
	{
		return m[x][y];
	}

	mat4_t getTranspose() const
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

	vec4_t<T> getRow(int32 row) const;

	vec4_t<T> getColumn(int32 column) const
	{
		return {m[0][column], m[1][column], m[2][column], m[3][column]};
	}

	void set(int32 x, int32 y, T value)
	{
		m[x][y] = value;
	}

	std::string toString() const
	{
		std::string output;

		output += std::format("[{} {} {} {}]\n", m[0][0], m[0][1], m[0][2], m[0][3]);
		output += std::format("[{} {} {} {}]\n", m[1][0], m[1][1], m[1][2], m[1][3]);
		output += std::format("[{} {} {} {}]\n", m[2][0], m[2][1], m[2][2], m[2][3]);
		output += std::format("[{} {} {} {}]\n", m[3][0], m[3][1], m[3][2], m[3][3]);

		return output;
	}

	mat4_t operator+(const mat4_t& other) const
	{
		mat4_t result;
		for (int32 x = 0; x < 4; x++)
		{
			for (int32 y = 0; y < 4; y++)
			{
				result.m[x][y] = m[x][y] + other.m[x][y];
			}
		}
		return result;
	}

	mat4_t& operator+=(const mat4_t& other)
	{
		*this = *this + other;
		return *this;
	}

	mat4_t operator-(const mat4_t& other) const
	{
		mat4_t result;
		for (int32 x = 0; x < 4; x++)
		{
			for (int32 y = 0; y < 4; y++)
			{
				result.m[x][y] = m[x][y] - other.m[x][y];
			}
		}
		return result;
	}

	mat4_t& operator-=(const mat4_t& other)
	{
		*this = *this - other;
		return *this;
	}

	mat4_t& operator*=(const mat4_t& other)
	{
		*this = *this * other;
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
		return *this;
	}

	vec3_t<T> operator*(const vec3_t<T>& v) const
	{
		vec3_t<T> result;
		for (int32 index = 0; index < 3; index++)
		{
			vec4_t<T> rowVector({m[index][0], m[index][1], m[index][2], m[index][3]});
			//result[index] = rowVector.dot(vec4_t<T>(v)); // TODO: Fix vec4f dot
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
constexpr mat4_t<T> operator*(const mat4_t<T>& m0, const mat4_t<T>& m1)
{
	using float4X4    = float[4][4];
	const float4X4& A = *((const float4X4*)m0.m);
	const float4X4& B = *((const float4X4*)m1.m);
	float4X4 temp;
	temp[0][0] = A[0][0] * B[0][0] + A[0][1] * B[1][0] + A[0][2] * B[2][0] + A[0][3] * B[3][0];
	temp[0][1] = A[0][0] * B[0][1] + A[0][1] * B[1][1] + A[0][2] * B[2][1] + A[0][3] * B[3][1];
	temp[0][2] = A[0][0] * B[0][2] + A[0][1] * B[1][2] + A[0][2] * B[2][2] + A[0][3] * B[3][2];
	temp[0][3] = A[0][0] * B[0][3] + A[0][1] * B[1][3] + A[0][2] * B[2][3] + A[0][3] * B[3][3];

	temp[1][0] = A[1][0] * B[0][0] + A[1][1] * B[1][0] + A[1][2] * B[2][0] + A[1][3] * B[3][0];
	temp[1][1] = A[1][0] * B[0][1] + A[1][1] * B[1][1] + A[1][2] * B[2][1] + A[1][3] * B[3][1];
	temp[1][2] = A[1][0] * B[0][2] + A[1][1] * B[1][2] + A[1][2] * B[2][2] + A[1][3] * B[3][2];
	temp[1][3] = A[1][0] * B[0][3] + A[1][1] * B[1][3] + A[1][2] * B[2][3] + A[1][3] * B[3][3];

	temp[2][0] = A[2][0] * B[0][0] + A[2][1] * B[1][0] + A[2][2] * B[2][0] + A[2][3] * B[3][0];
	temp[2][1] = A[2][0] * B[0][1] + A[2][1] * B[1][1] + A[2][2] * B[2][1] + A[2][3] * B[3][1];
	temp[2][2] = A[2][0] * B[0][2] + A[2][1] * B[1][2] + A[2][2] * B[2][2] + A[2][3] * B[3][2];
	temp[2][3] = A[2][0] * B[0][3] + A[2][1] * B[1][3] + A[2][2] * B[2][3] + A[2][3] * B[3][3];

	temp[3][0] = A[3][0] * B[0][0] + A[3][1] * B[1][0] + A[3][2] * B[2][0] + A[3][3] * B[3][0];
	temp[3][1] = A[3][0] * B[0][1] + A[3][1] * B[1][1] + A[3][2] * B[2][1] + A[3][3] * B[3][1];
	temp[3][2] = A[3][0] * B[0][2] + A[3][1] * B[1][2] + A[3][2] * B[2][2] + A[3][3] * B[3][2];
	temp[3][3] = A[3][0] * B[0][3] + A[3][1] * B[1][3] + A[3][2] * B[2][3] + A[3][3] * B[3][3];

	mat4_t<T> result;
	memcpy(&result.m, &temp, 16 * sizeof(float));
	return result;
}

template <typename T>
struct mat4_persp_t : mat4_t<T>
{
	mat4_persp_t(T fov, T aspect, T minZ, T maxZ = MAX_Z) : mat4_t<T>()
	{
		this->setZero();
		const T tanHalfFov = std::tanf(fov / 2.0f);
		this->m[0][0]      = T(1) / (aspect * tanHalfFov);
		this->m[1][1]      = T(1) / tanHalfFov;

		this->m[2][2] = -(maxZ + minZ) / (maxZ - minZ);
		this->m[3][2] = -T(1);
		this->m[2][3] = -(T(2) * maxZ * minZ) / (maxZ - minZ);
	}
};

template <typename T>
struct mat4_lookat_t : mat4_t<T>
{
	mat4_lookat_t(const vec3_t<T>& eye, const vec3_t<T>& center, const vec3_t<T>& upVector) : mat4_t<T>()
	{
		// Forward vector
		const vec3_t<T> forward = (center - eye).normalized();

		// Right vector
		const vec3_t<T> right = forward.cross(upVector).normalized();

		// Up vector
		const vec3_t<T> up = right.cross(forward);

		//  Rx |  Ux | -Fx | -Tx
		//  Ry |  Uy | -Fy | -Ty
		//  Rz |  Uz | -Fz | -Tz 
		//  0  |  0  |  0  |  1
		this->setIdentity();
		this->m[0][0] = right.x;
		this->m[0][1] = right.y;
		this->m[0][2] = right.z;
		this->m[1][0] = up.x;
		this->m[1][1] = up.y;
		this->m[1][2] = up.z;
		this->m[2][0] = -forward.x;
		this->m[2][1] = -forward.y;
		this->m[2][2] = -forward.z;
		this->m[0][3] = -up.dot(eye);
		this->m[1][3] = -right.dot(eye);
		this->m[2][3] = forward.dot(eye);
	}
};

template <typename T>
struct mat4_trans_t : mat4_t<T>
{
	mat4_trans_t(const vec3_t<T>& delta) : mat4_t<T>()
	{
		this->m[3][0] = delta.x;
		this->m[3][1] = delta.y;
		this->m[3][2] = delta.z;
	}
};

template <typename T>
struct mat4_rot_t : mat4_t<T>
{
	mat4_rot_t(T pitch, T yaw, T roll) : mat4_t<T>()
	{
		// Convert from degrees to radians
		pitch = Math::degreesToRadians(pitch);
		yaw   = Math::degreesToRadians(yaw);
		roll  = Math::degreesToRadians(roll);

		T cp = std::cosf(pitch);
		T sp = std::sinf(pitch);
		T cy = std::cosf(yaw);
		T sy = std::sinf(yaw);
		T cr = std::cosf(roll);
		T sr = std::sinf(roll);

		T cpsy = cp * sy;
		T spsy = sp * sy;

		this->m[0][0] = cy * cr;
		this->m[0][1] = -cy * sr;
		this->m[0][2] = sy;
		this->m[1][0] = spsy * cr + cp * sr;
		this->m[1][1] = -spsy * sr + cp * cr;
		this->m[1][2] = -sp * cy;
		this->m[2][0] = -cpsy * cr + sp * sr;
		this->m[2][1] = cpsy * sr + sp * cr;
		this->m[2][2] = cp * cy;
	}

	mat4_rot_t(const rot_t<T>& rotation) : mat4_t<T>()
	{
		*this = mat4_rot_t(rotation.pitch, rotation.yaw, rotation.roll);
	}
};

template <typename T>
struct mat4_rottrans_t : mat4_t<T>
{
	mat4_rottrans_t(const rot_t<T>& rotation, const vec3_t<T>& translation) : mat4_t<T>()
	{
		mat4_t<T> rotationMatrix    = mat4_rot_t<T>(rotation);
		mat4_t<T> translationMatrix = mat4_trans_t<T>(translation);
		*this                       = rotationMatrix * translationMatrix;
	}
};
