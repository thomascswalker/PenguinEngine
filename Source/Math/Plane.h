#pragma once

template <typename T>
struct plane_t
{
	union
	{
		struct // NOLINT
		{
			T x;
			T y;
			T z;
			T w;
		};

		T xyzw[4];
	};

	plane_t(T v[4]) : x(v[0]), y(v[1]), z(v[2]), w(v[3])
	{
	}

	plane_t(T inX, T inY, T inZ, T inW) : x(inX), y(inY), z(inZ), w(inW)
	{
	}

	plane_t operator*(T scalar) const
	{
		plane_t out(*this);
		out.x *= scalar;
		out.y *= scalar;
		out.z *= scalar;
		out.w *= scalar;
		return out;
	}

	plane_t& operator*(T scalar)
	{
		*this = *this * scalar;
		return *this;
	}

	T operator[](int32 index) const { return xyzw[index]; }
	T& operator[](int32 index) { return xyzw[index]; }
};
