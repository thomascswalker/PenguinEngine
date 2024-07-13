#pragma once

#include "Vector.h"

inline void VecAddVec(const FVector4& V0, const FVector4& V1, FVector4& Out)
{
    const __m128 A = _mm_load_ps(V0.XYZW);
    const __m128 B = _mm_load_ps(V1.XYZW);
    const __m128 C = _mm_add_ps(A, B);
    _mm_store_ps(Out.XYZW, C);
}

inline void VecSubVec(const FVector4& V0, const FVector4& V1, FVector4& Out)
{
    const __m128 A = _mm_load_ps(V0.XYZW);
    const __m128 B = _mm_load_ps(V1.XYZW);
    const __m128 C = _mm_sub_ps(A, B);
    _mm_store_ps(Out.XYZW, C);
}

inline void VecMulVec(const FVector4& V0, const FVector4& V1, FVector4& Out)
{
    const __m128 A = _mm_load_ps(V0.XYZW);
    const __m128 B = _mm_load_ps(V1.XYZW);
    const __m128 C = _mm_mul_ps(A, B);
    _mm_store_ps(Out.XYZW, C);
}

inline void VecDivVec(const FVector4& V0, const FVector4& V1, FVector4& Out)
{
    const __m128 A = _mm_load_ps(V0.XYZW);
    const __m128 B = _mm_load_ps(V1.XYZW);
    const __m128 C = _mm_div_ps(A, B);
    _mm_store_ps(Out.XYZW, C);
}

inline void VecDotVec(const FVector4& V0, const FVector4& V1, float* Out)
{
    FVector4 V2;
    VecMulVec(V0, V1, V2);
    *Out = V2.X + V2.Y + V2.Z;
}

// https://geometrian.com/programming/tutorials/cross-product/index.php
inline void VecCrossVec(const FVector4& V0, const FVector4& V1, FVector4& Out)
{
    const __m128 A = _mm_load_ps(V0.XYZW);
    const __m128 B = _mm_load_ps(V1.XYZW);

    __m128 Tmp0 = _mm_shuffle_ps(A, A,_MM_SHUFFLE(3, 0, 2, 1));
    __m128 Tmp1 = _mm_shuffle_ps(B, B,_MM_SHUFFLE(3, 1, 0, 2));
    __m128 Tmp2 = _mm_shuffle_ps(A, A,_MM_SHUFFLE(3, 1, 0, 2));
    __m128 Tmp3 = _mm_shuffle_ps(B, B,_MM_SHUFFLE(3, 0, 2, 1));

    __m128 Result = _mm_sub_ps(
        _mm_mul_ps(Tmp0, Tmp1),
        _mm_mul_ps(Tmp2, Tmp3)
    );
    _mm_store_ps(Out.XYZW, Result);
}
