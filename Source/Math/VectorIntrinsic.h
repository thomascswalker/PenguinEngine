#pragma once

#include <DirectXMath.h>
#include <xmmintrin.h>

#include "Vector.h"

struct vecmu32
{
	union
	{
		uint32 u[4];
		__m128 v;
	};
};

constexpr vecmu32 g_mask3 = {UINT32_MAX, UINT32_MAX, UINT32_MAX, 0};

// ReSharper disable once CppInconsistentNaming
struct vecm
{
	__m128 m;

	/** Constructors **/

	vecm()
		: m(_mm_set_ps(0, 0, 0, 0)) {}

	vecm(const __m128 value)
		: m(value) {}

	vecm(const float v)
		: m(_mm_set_ps(v, v, v, v)) {}

	vecm(const float x, const float y, const float z)
		: m(_mm_set_ps(x, y, z, 0)) {}

	vecm(const vec3f& v)
		: m(_mm_set_ps(v.x, v.y, v.z, 0)) {}

	vecm(const float x, const float y, const float z, const float w)
		: m(_mm_set_ps(x, y, z, w)) {}

	vecm(const vec4f& v)
		: m(_mm_set_ps(v.x, v.y, v.z, v.w)) {}

	vecm(const float* values)
		: m(_mm_load_ps(values)) {}

	/** General methods **/

	[[nodiscard]] float x() const
	{
		float out;
		_MM_EXTRACT_FLOAT(out, m, 0);
		return out;
	}

	[[nodiscard]] float y() const
	{
		float out;
		_MM_EXTRACT_FLOAT(out, m, 1);
		return out;
	}

	[[nodiscard]] float z() const
	{
		float out;
		_MM_EXTRACT_FLOAT(out, m, 2);
		return out;
	}

	[[nodiscard]] float w() const
	{
		float out;
		_MM_EXTRACT_FLOAT(out, m, 3);
		return out;
	}

	/** 3D Vector **/

	[[nodiscard]] vecm dot3(const vecm& other) const
	{
		// Perform the dot product
		__m128 vDot = _mm_mul_ps(m, other.m);
		// x=Dot.vector4_f32[1], y=Dot.vector4_f32[2]
		__m128 vTemp = _mm_shuffle_ps((vDot), (vDot), _MM_SHUFFLE(2, 1, 2, 1));
		// Result.vector4_f32[0] = x+y
		vDot = _mm_add_ss(vDot, vTemp);
		// x=Dot.vector4_f32[2]
		vTemp = _mm_shuffle_ps((vTemp), (vTemp), _MM_SHUFFLE(1, 1, 1, 1));
		// Result.vector4_f32[0] = (x+y)+z
		vDot = _mm_add_ss(vDot, vTemp);
		// Splat x
		return _mm_shuffle_ps((vDot), (vDot), _MM_SHUFFLE(0, 0, 0, 0));
	}

	vecm cross3(const vecm& other) const
	{
		// y1,z1,x1,w1
		__m128 vTemp1 = _mm_shuffle_ps((m), (m), _MM_SHUFFLE(3, 0, 2, 1));
		// z2,x2,y2,w2
		__m128 vTemp2 = _mm_shuffle_ps((other.m), (other.m), _MM_SHUFFLE(3, 1, 0, 2));
		// Perform the left operation
		__m128 vResult = _mm_mul_ps(vTemp1, vTemp2);
		// z1,x1,y1,w1
		vTemp1 = _mm_shuffle_ps((vTemp1), (vTemp1), _MM_SHUFFLE(3, 0, 2, 1));
		// y2,z2,x2,w2
		vTemp2 = _mm_shuffle_ps((vTemp2), (vTemp2), _MM_SHUFFLE(3, 1, 0, 2));
		// Perform the right operation
		vResult = _mm_sub_ps(vResult, _mm_mul_ps(vTemp1, vTemp2));
		// Set w to zero
		return _mm_and_ps(vResult, g_mask3.v);
	}

	[[nodiscard]] vecm length3() const
	{
		// Perform the dot product on x,y and z
		__m128 vLengthSq = _mm_mul_ps(m, m);
		// vTemp has z and y
		__m128 vTemp = _mm_shuffle_ps(vLengthSq, vLengthSq, _MM_SHUFFLE(1, 2, 1, 2));
		// x+z, y
		vLengthSq = _mm_add_ss(vLengthSq, vTemp);
		// y,y,y,y
		vTemp = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
		// x+z+y,??,??,??
		vLengthSq = _mm_add_ss(vLengthSq, vTemp);
		// Splat the length squared
		vLengthSq = _mm_shuffle_ps(vLengthSq, vLengthSq, _MM_SHUFFLE(0, 0, 0, 0));
		// Get the length
		return _mm_sqrt_ps(vLengthSq);
	}

	/** 4D Vector **/

	[[nodiscard]] vecm length4() const
	{
		// Perform the dot product on x,y,z and w
		__m128 vLengthSq = _mm_mul_ps(m, m);
		// vTemp has z and w
		__m128 vTemp = _mm_shuffle_ps((vLengthSq), (vLengthSq), _MM_SHUFFLE(3, 2, 3, 2));
		// x+z, y+w
		vLengthSq = _mm_add_ps(vLengthSq, vTemp);
		// x+z,x+z,x+z,y+w
		vLengthSq = _mm_shuffle_ps((vLengthSq), (vLengthSq), _MM_SHUFFLE(1, 0, 0, 0));
		// ??,??,y+w,y+w
		vTemp = _mm_shuffle_ps(vTemp, vLengthSq, _MM_SHUFFLE(3, 3, 0, 0));
		// ??,??,x+z+y+w,??
		vLengthSq = _mm_add_ps(vLengthSq, vTemp);
		// Splat the length
		vLengthSq = _mm_shuffle_ps((vLengthSq), (vLengthSq), _MM_SHUFFLE(2, 2, 2, 2));
		// Get the length
		vLengthSq = _mm_sqrt_ps(vLengthSq);
		return vLengthSq;
	}

	/** Operators **/

	vecm operator+(const vecm& other) const
	{
		return _mm_add_ps(m, other.m);
	}

	vecm& operator+=(const vecm& other)
	{
		m = _mm_add_ps(m, other.m);
		return *this;
	}

	vecm operator-(const vecm& other) const
	{
		return _mm_sub_ps(m, other.m);
	}

	vecm& operator-=(const vecm& other)
	{
		m = _mm_sub_ps(m, other.m);
		return *this;
	}

	vecm operator*(const vecm& other) const
	{
		return _mm_mul_ps(m, other.m);
	}

	vecm& operator*=(const vecm& other)
	{
		m = _mm_mul_ps(m, other.m);
		return *this;
	}

	vecm operator/(const vecm& other) const
	{
		return _mm_div_ps(m, other.m);
	}

	vecm operator/(float x) const
	{
		vecm tmp(x);
		return _mm_div_ps(m, tmp.m);
	}

	vecm& operator/=(const vecm& other)
	{
		m = _mm_div_ps(m, other.m);
		return *this;
	}

	explicit operator float() const
	{
		return _mm_cvtss_f32(m);
	}

	explicit operator vec3f() const
	{
		vec3f out;
		_MM_EXTRACT_FLOAT(out.x, m, 0);
		_MM_EXTRACT_FLOAT(out.y, m, 1);
		_MM_EXTRACT_FLOAT(out.z, m, 2);
		return out;
	}

	explicit operator vec4f() const
	{
		vec4f out;
		_MM_EXTRACT_FLOAT(out.x, m, 0);
		_MM_EXTRACT_FLOAT(out.y, m, 1);
		_MM_EXTRACT_FLOAT(out.z, m, 2);
		_MM_EXTRACT_FLOAT(out.w, m, 2);
		return out;
	}
};

struct matm
{
	__m128 m0;
	__m128 m1;
	__m128 m2;
	__m128 m3;

	matm(const mat4f& mat)
	{
		m0 = _mm_load_ps(&mat.m[0][0]); // Row 1
		m1 = _mm_load_ps(&mat.m[1][0]); // Row 2
		m2 = _mm_load_ps(&mat.m[2][0]); // Row 3
		m3 = _mm_load_ps(&mat.m[3][0]); // Row 4
	}

	vecm operator*(const vecm& v) const
	{
		__m128 vResult = _mm_shuffle_ps(v.m, v.m, _MM_SHUFFLE(3, 3, 3, 3)); // W
		vResult        = _mm_mul_ps(vResult, m3);
		__m128 vTemp   = _mm_shuffle_ps(v.m, v.m, _MM_SHUFFLE(2, 2, 2, 2)); // Z
		vResult        = _mm_add_ps(_mm_mul_ps(vTemp, m2), vResult);
		vTemp          = _mm_shuffle_ps(v.m, v.m, _MM_SHUFFLE(1, 1, 1, 1)); // Y
		vResult        = _mm_add_ps(_mm_mul_ps(vTemp, m1), vResult);
		vTemp          = _mm_shuffle_ps(v.m, v.m, _MM_SHUFFLE(0, 0, 0, 0)); // X
		vResult        = _mm_add_ps(_mm_mul_ps(vTemp, m0), vResult);
		return vResult;
	}
};

inline void vecLoad(const vec4f& in, float* out)
{
	memcpy(out, &in, 32);
}

inline void vecUnload(const float* in, vec4f* out)
{
	out->x = in[0];
	out->y = in[1];
	out->z = in[2];
	out->w = in[3];
}

inline void vecAddVec(const vec4f& v0, const vec4f& v1, vec4f& out)
{
	const __m128 a = _mm_load_ps(v0.xyzw);
	const __m128 b = _mm_load_ps(v1.xyzw);
	const __m128 c = _mm_add_ps(a, b);
	_mm_store_ps(out.xyzw, c);
}

inline void vecSubVec(const vec4f& v0, const vec4f& v1, vec4f& out)
{
	const __m128 a = _mm_load_ps(v0.xyzw);
	const __m128 b = _mm_load_ps(v1.xyzw);
	const __m128 c = _mm_sub_ps(a, b);
	_mm_store_ps(out.xyzw, c);
}

inline void vecMulVec(const vec4f& v0, const vec4f& v1, vec4f& out)
{
	const __m128 a = _mm_load_ps(v0.xyzw);
	const __m128 b = _mm_load_ps(v1.xyzw);
	const __m128 c = _mm_mul_ps(a, b);
	_mm_store_ps(out.xyzw, c);
}

inline void vecDivVec(const vec4f& v0, const vec4f& v1, vec4f& out)
{
	const __m128 a = _mm_load_ps(v0.xyzw);
	const __m128 b = _mm_load_ps(v1.xyzw);
	const __m128 c = _mm_div_ps(a, b);
	_mm_store_ps(out.xyzw, c);
}

inline void vecDotVec(const vec4f& v0, const vec4f& v1, float* out)
{
	vec4f v2;
	vecMulVec(v0, v1, v2);
	*out = v2.x + v2.y + v2.z;
}

// https://geometrian.com/programming/tutorials/cross-product/index.php
inline void vecCrossVec(const vec4f& v0, const vec4f& v1, vec4f& out)
{
	const __m128 a = _mm_load_ps(v0.xyzw);
	const __m128 b = _mm_load_ps(v1.xyzw);

	__m128 tmp0 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 tmp1 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 tmp2 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 tmp3 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));

	__m128 result = _mm_sub_ps(
		_mm_mul_ps(tmp0, tmp1),
		_mm_mul_ps(tmp2, tmp3));
	_mm_store_ps(out.xyzw, result);
}
