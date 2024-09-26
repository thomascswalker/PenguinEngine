// ReSharper disable CppInconsistentNaming
#pragma once

#include <DirectXMath.h>

#include "Math/Matrix.h"
#include "Math/Vector.h"

using namespace DirectX;

inline XMVECTOR toXMVector(const vec3f& v, const float w = 0.0f)
{
	return XMVectorSet(v.x, v.y, v.z, w);
}

inline XMFLOAT3 toXMFloat3(const vec3f& v)
{
	return XMFLOAT3(v.xyz);
}

inline XMMATRIX toXMMatrix(const mat4f& m)
{
	return XMMATRIX(&m.m[0][0]);
}
