﻿#pragma once

#include <xmmintrin.h>
#include "Vector.h"

inline void vecLoad(const vec4f& in, float* out)
{
	memcpy(out, &in, 32);
}

inline void vecUnload(const float* in, vec4f* out)
{
	out->X = in[0];
	out->Y = in[1];
	out->Z = in[2];
	out->W = in[3];
}

inline void vecAddVec(const vec4f& v0, const vec4f& v1, vec4f& out)
{
	const __m128 a = _mm_load_ps(v0.XYZW);
	const __m128 b = _mm_load_ps(v1.XYZW);
	const __m128 c = _mm_add_ps(a, b);
	_mm_store_ps(out.XYZW, c);
}

inline void vecSubVec(const vec4f& v0, const vec4f& v1, vec4f& out)
{
	const __m128 a = _mm_load_ps(v0.XYZW);
	const __m128 b = _mm_load_ps(v1.XYZW);
	const __m128 c = _mm_sub_ps(a, b);
	_mm_store_ps(out.XYZW, c);
}

inline void vecMulVec(const vec4f& v0, const vec4f& v1, vec4f& out)
{
	const __m128 a = _mm_load_ps(v0.XYZW);
	const __m128 b = _mm_load_ps(v1.XYZW);
	const __m128 c = _mm_mul_ps(a, b);
	_mm_store_ps(out.XYZW, c);
}

inline void vecDivVec(const vec4f& v0, const vec4f& v1, vec4f& out)
{
	const __m128 a = _mm_load_ps(v0.XYZW);
	const __m128 b = _mm_load_ps(v1.XYZW);
	const __m128 c = _mm_div_ps(a, b);
	_mm_store_ps(out.XYZW, c);
}

inline void vecDotVec(const vec4f& v0, const vec4f& v1, float* out)
{
	vec4f v2;
	vecMulVec(v0, v1, v2);
	*out = v2.X + v2.Y + v2.Z;
}

// https://geometrian.com/programming/tutorials/cross-product/index.php
inline void vecCrossVec(const vec4f& v0, const vec4f& v1, vec4f& out)
{
	const __m128 a = _mm_load_ps(v0.XYZW);
	const __m128 b = _mm_load_ps(v1.XYZW);

	__m128 tmp0 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1));
	__m128 tmp1 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 tmp2 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 1, 0, 2));
	__m128 tmp3 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1));

	__m128 result = _mm_sub_ps(
		_mm_mul_ps(tmp0, tmp1),
		_mm_mul_ps(tmp2, tmp3)
	);
	_mm_store_ps(out.XYZW, result);
}
