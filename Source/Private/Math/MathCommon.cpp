#include "Math/MathCommon.h"
#include "Framework/Renderer/Camera.h"

// Matrices

template <typename T>
vec3_t<T> mat4_t<T>::getScale(T tolerance)
{
	vec3_t<T> scale(0, 0, 0);

	// For each row, find magnitude, and if its non-zero re-scale so its unit length.
	const T squareSum0 = (m[0][0] * m[0][0]) + (m[0][1] * m[0][1]) + (m[0][2] * m[0][2]);
	const T squareSum1 = (m[1][0] * m[1][0]) + (m[1][1] * m[1][1]) + (m[1][2] * m[1][2]);
	const T squareSum2 = (m[2][0] * m[2][0]) + (m[2][1] * m[2][1]) + (m[2][2] * m[2][2]);

	if (squareSum0 > tolerance)
	{
		T scale0 = std::sqrtf(squareSum0);
		scale[0] = scale0;
		T invScale0 = 1.f / scale0;
		m[0][0] *= invScale0;
		m[0][1] *= invScale0;
		m[0][2] *= invScale0;
	}
	else
	{
		scale[0] = 0;
	}

	if (squareSum1 > tolerance)
	{
		T scale1 = std::sqrtf(squareSum1);
		scale[1] = scale1;
		T invScale1 = 1.f / scale1;
		m[1][0] *= invScale1;
		m[1][1] *= invScale1;
		m[1][2] *= invScale1;
	}
	else
	{
		scale[1] = 0;
	}

	if (squareSum2 > tolerance)
	{
		T scale2 = std::sqrtf(squareSum2);
		scale[2] = scale2;
		T invScale2 = 1.f / scale2;
		m[2][0] *= invScale2;
		m[2][1] *= invScale2;
		m[2][2] *= invScale2;
	}
	else
	{
		scale[2] = 0;
	}

	return scale;
}

template <typename T>
vec4_t<T> mat4_t<T>::getRow(int32 row) const
{
	return {m[row][0], m[row][1], m[row][2], m[row][3]};
}

// Rotator

template <typename T>
rot_t<T>::rot_t(const vec3_t<T>& euler)
{
	pitch = euler.y;
	yaw = euler.z;
	roll = euler.x;
}

// Instantiate for the linker
template struct rot_t<float>;
template struct rot_t<double>;
template struct quat_t<float>;
template struct quat_t<double>;
template struct vec3_t<float>;
template struct vec3_t<double>;
template struct vec4_t<float>;
template struct vec4_t<double>;
template struct mat4_t<float>;
template struct mat4_t<double>;
