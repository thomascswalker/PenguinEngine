// ReSharper disable CppInconsistentNaming
#pragma once

#include <format>
#include <cassert>
#include <DirectXMath.h>
#include <intrin.h>

#include "Core/Logging.h"
#include "Math.h"

#define EDGE_FUNCTION(X0, Y0, X1, Y1, X2, Y2) (((X1) - (X0)) * ((Y2) - (Y0)) - ((Y1) - (Y0)) * ((X2) - (X0)))

template <typename T>
struct vec2_t
{
	static_assert(std::is_arithmetic_v<T>, "Type is not a number.");

	// Memory aligned coordinate values
	union
	{
		struct // NOLINT(clang-diagnostic-nested-anon-types)
		{
			T x;
			T y;
		};

		T xy[2] = {};
	};

	// Constructors
	vec2_t()
		: x(0), y(0) {}

	vec2_t(T inX)
		: x(inX), y(inX) {}

	vec2_t(T inX, T inY)
		: x(inX), y(inY) {}

	vec2_t(const std::initializer_list<T>& values)
	{
		x = *(values.begin());
		y = *(values.begin() + 1);
	}

	// Functions
	static vec2_t zeroVector()
	{
		return vec2_t();
	}

	static vec2_t identityVector()
	{
		return vec2_t(1);
	}

	template <typename ToType>
	vec2_t<ToType> toType() const
	{
		return {static_cast<ToType>(x), static_cast<ToType>(y)};
	}

	T length()
	{
		return x * x + y * y;
	}

	void normalize()
	{
		T len = length();
		x /= len;
		y /= len;
	}

	vec2_t normalized() const
	{
		T len = length();
		return {x / len, y / len};
	}

	std::string toString() const
	{
		return std::format("[{}, {}]", x, y);
	}

	void zero()
	{
		x = 0.0f;
		y = 0.0f;
	}

	// Operators
	vec2_t operator+(const vec2_t& v) const
	{
		return {x + v.x, y + v.y};
	}

	vec2_t& operator+=(const vec2_t& v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	vec2_t operator-(const vec2_t& v) const
	{
		return {x - v.x, y - v.y};
	}

	vec2_t& operator-=(const vec2_t& v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	vec2_t operator*(const vec2_t& v) const
	{
		return {x * v.x, y * v.y};
	}

	vec2_t& operator*=(const vec2_t& v)
	{
		x *= v.x;
		y *= v.y;
		return *this;
	}

	vec2_t operator/(const vec2_t& v) const
	{
		return {x / v.x, y / v.y};
	}

	vec2_t& operator/=(const vec2_t& v)
	{
		x /= v.x;
		y /= v.y;
		return *this;
	}

	vec2_t operator-()
	{
		return vec2_t(-x, -y);
	}

	bool operator<(const vec2_t& other) const
	{
		return x < other.x && y < other.y;
	}

	bool operator>(const vec2_t& other) const
	{
		return x > other.x && y > other.y;
	}

	bool operator>(T value)
	{
		return x > value && y > value;
	}

	bool operator<(T value)
	{
		return x < value && y < value;
	}

	bool operator==(T value)
	{
		return x == value && y == value;
	}

	bool operator==(const vec2_t& other)
	{
		return x == other.x && y == other.y;
	}

	bool operator!=(const vec2_t& other)
	{
		return x != other.x || y != other.y;
	}

	T operator[](int32 index) const
	{
		return xy[index];
	}

	T& operator[](int32 index)
	{
		return xy[index];
	}
};

template <typename T>
struct vec3_t
{
	static_assert(std::is_arithmetic_v<T>, "Type is not a number.");

	// Memory aligned coordinate values
	union
	{
		struct // NOLINT(clang-diagnostic-nested-anon-types)
		{
			T x;
			T y;
			T z;
		};

		T xyz[3];
	};

	// Constructors
	vec3_t()
		: x(0), y(0), z(0) {}

	vec3_t(T inX)
		: x(inX), y(inX), z(inX) {}

	vec3_t(T inX, T inY, T inZ)
		: x(inX), y(inY), z(inZ) {}

	vec3_t(const vec2_t<T>& v, T inZ = T(1))
		: x(v.x), y(v.y), z(inZ) {}

	vec3_t(const std::initializer_list<T>& values)
	{
		x = *(values.begin());
		y = *(values.begin() + 1);
		z = *(values.begin() + 2);
	}

	// Functions
	static vec3_t zeroVector()
	{
		return vec3_t();
	}

	static vec3_t identityVector()
	{
		return vec3_t(1);
	}

	static vec3_t forwardVector()
	{
		return vec3_t(1, 0, 0);
	}

	static vec3_t backVector()
	{
		return vec3_t(-1, 0, 0);
	}

	static vec3_t upVector()
	{
		return vec3_t(0, 1, 0);
	}

	static vec3_t downVector()
	{
		return vec3_t(0, -1, 0);
	}

	static vec3_t rightVector()
	{
		return vec3_t(0, 0, 1);
	}

	static vec3_t leftVector()
	{
		return vec3_t(0, 0, -1);
	}

	template <typename ToType>
	vec3_t<ToType> toType() const
	{
		return vec3_t<ToType>{static_cast<ToType>(x), static_cast<ToType>(y), static_cast<ToType>(z)};
	}

	void zero()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	void normalize()
	{
#ifndef PENG_SSE
		const T magnitude = length();
		if (magnitude < 0.000001f)
		{
			*this = zeroVector();
		}
		else
		{
			x /= magnitude;
			y /= magnitude;
			z /= magnitude;
		}
#else
		// https://fastcpp.blogspot.com/2012/02/calculating-length-of-3d-vector-using.html
		__m128 vec        = _mm_setr_ps((float)x, (float)y, (float)z, 0.0f); // Convert to __m128
		__m128 dotProduct = _mm_dp_ps(vec, vec, g_maskW); // Compute dot product
		__m128 invNormal  = _mm_rsqrt_ps(dotProduct); // Get inverse normal
		vec               = _mm_mul_ps(vec, invNormal); // Multiply original vector by the inverse normal

		// Reassign XYZ
		_MM_EXTRACT_FLOAT(x, vec, 0);
		_MM_EXTRACT_FLOAT(y, vec, 1);
		_MM_EXTRACT_FLOAT(z, vec, 2);
#endif //  _INCLUDED_MM2
	}

	vec3_t normalized() const
	{
		vec3_t out(x, y, z);
		out.normalize();
		return out;
	}

	constexpr T length() const
	{
#ifndef PENG_SSE
		return std::sqrtf(x * x + y * y + z * z);
#else
		__m128 vec = _mm_setr_ps((float)x, (float)y, (float)z, 0.0f);
		// Compute length via dot product, masking the final two bits as we don't care about the W component
		// Initialize storage
		__m128 dotProduct = _mm_dp_ps(vec, vec, 0x7F); // 0111_1111
		// Compute the reciprocal square root of the dot product
		__m128 invSqrt = _mm_sqrt_ps(dotProduct);
		// Convert to float
		float result = _mm_cvtss_f32(invSqrt);
		return (T)result;
#endif
	}

	/** https://geometrian.com/programming/tutorials/cross-product/index.php **/
	vec3_t cross(const vec3_t& v) const
	{
#ifndef PENG_SSE
		return vec3_t{
			y * v.z - z * v.y,
			x * v.z - z * v.x,
			x * v.y - y * v.x
		};
#else
		__m128 V1 = toM128();
		__m128 V2 = v.toM128();
		// y1,z1,x1,w1
		__m128 vTemp1 = XM_PERMUTE_PS(V1, _MM_SHUFFLE(3, 0, 2, 1));
		// z2,x2,y2,w2
		__m128 vTemp2 = XM_PERMUTE_PS(V2, _MM_SHUFFLE(3, 1, 0, 2));
		// Perform the left operation
		__m128 vResult = _mm_mul_ps(vTemp1, vTemp2);
		// z1,x1,y1,w1
		vTemp1 = XM_PERMUTE_PS(vTemp1, _MM_SHUFFLE(3, 0, 2, 1));
		// y2,z2,x2,w2
		vTemp2 = XM_PERMUTE_PS(vTemp2, _MM_SHUFFLE(3, 1, 0, 2));
		// Perform the right operation
		vResult = XM_FNMADD_PS(vTemp1, vTemp2, vResult);
		// Set w to zero
		vec3_t out;
		__m128 r = _mm_and_ps(vResult, DirectX::g_XMMask3);
		_MM_EXTRACT_FLOAT(out.x, r, 0);
		_MM_EXTRACT_FLOAT(out.y, r, 1);
		_MM_EXTRACT_FLOAT(out.z, r, 2);
		return out;
#endif
	}

	T dot(const vec3_t& v) const
	{
#ifndef PENG_SSE
		T result = 0;
		for (int32 index = 0; index < 3; index++)
		{
			result += xyz[index] * v[index];
		}
		return result;
#else
		__m128 a = toM128();
		__m128 b = v.toM128();
		// Compute dot product
		__m128 dotProduct = _mm_dp_ps(a, b, 0x7F); // 0111_1111
		// Extract the lowest 32-bit float
		return (T)_mm_cvtss_f32(dotProduct);
#endif
	}

	vec3_t swizzleXY() const
	{
		return {y, x, z};
	}

	vec3_t swizzleXZ() const
	{
		return {z, y, x};
	}

	vec3_t swizzleYZ() const
	{
		return {x, z, y};
	}

	vec3_t swizzleXYZ() const
	{
		return {z, x, y};
	}

	__m128 toM128() const
	{
		// Convert to float array in reverse order as elements are loaded right to left
		float _xyzw[4] = {(T)x, (T)y, (T)z, 0.0f};
		// Store the float array
		return _mm_load_ps(_xyzw);
	}

	std::string toString() const
	{
		return std::format("[{}, {}, {}]", x, y, z);
	}

	// Operators
	vec3_t operator+(const vec3_t& v) const
	{
		return {x + v.x, y + v.y, z + v.z};
	}

	vec3_t& operator+=(const vec3_t& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	vec3_t operator-(const vec3_t& v) const
	{
		return {x - v.x, y - v.y, z - v.z};
	}

	vec3_t& operator-=(const vec3_t& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	vec3_t operator*(const vec3_t& v) const
	{
		return {x * v.x, y * v.y, z * v.z};
	}

	vec3_t& operator*=(const vec3_t& v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	vec3_t operator/(const vec3_t& v) const
	{
		return {x / v.x, y / v.y, z / v.z};
	}

	vec3_t& operator/=(const vec3_t& v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}

	bool operator==(const vec3_t& v) const
	{
		return x == v.x && y == v.y && z == v.z;
	}

	bool operator!=(const vec3_t& v) const
	{
		return x != v.x || y != v.y || z != v.z;
	}

	vec3_t operator-() const
	{
		return vec3_t(-x, -y, -z);
	}

	bool operator<(const vec3_t& other)
	{
		return x < other.x && y < other.y && z < other.z;
	}

	bool operator>(const vec3_t& other)
	{
		return x > other.x && y > other.y && z > other.z;
	}

	T operator[](int32 index) const
	{
		return xyz[index];
	}

	T& operator[](int32 index)
	{
		return xyz[index];
	}

	operator vec2_t<T>() const
	{
		return {x, y};
	}
};

template <typename T>
struct vec4_t
{
	static_assert(std::is_arithmetic_v<T>, "Type is not a number.");

	// Memory aligned coordinate values
	union
	{
		struct // NOLINT(clang-diagnostic-nested-anon-types)
		{
			T x;
			T y;
			T z;
			T w;
		};

		T xyzw[4];
	};

	// Constructors
	vec4_t()
		: x(0), y(0), z(0), w(0) {}

	vec4_t(T inX)
		: x(inX), y(inX), z(inX), w(inX) {}

	vec4_t(T inX, T inY, T inZ, T inW)
		: x(inX), y(inY), z(inZ), w(inW) {}

	vec4_t(T* values)
		: x(values[0]), y(values[1]), z(values[2]), w(values[3]) {}

	vec4_t(const std::initializer_list<T>& values)
	{
		x = *(values.begin());
		y = *(values.begin() + 1);
		z = *(values.begin() + 2);
		w = *(values.begin() + 3);
	}

	vec4_t(const vec3_t<T>& v, T inW = T(1))
		: x(v.x), y(v.y), z(v.z), w(inW) {}

	// Functions
	static vec4_t zeroVector()
	{
		return vec4_t();
	}

	static vec4_t identityVector()
	{
		return vec4_t(1);
	}

	template <typename ToType>
	vec4_t<ToType> toType() const
	{
		return {static_cast<ToType>(x), static_cast<ToType>(y), static_cast<ToType>(z), static_cast<ToType>(w)};
	}

	std::string toString() const
	{
		return std::format("[{}, {}, {}, {}]", x, y, z, w);
	}

	void zero()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 0.0f;
	}

	// Operators
	vec4_t operator+(const vec4_t& v) const
	{
		return {x + v.x, y + v.y, z + v.z, w + v.w};
	}

	vec4_t& operator+=(const vec4_t& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	vec4_t operator-(const vec4_t& v) const
	{
		return {x - v.x, y - v.y, z - v.z, w - v.w};
	}

	vec4_t& operator-=(const vec4_t& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	vec4_t operator*(const vec4_t& v) const
	{
		return {x * v.x, y * v.y, z * v.z, w * v.w};
	}

	vec4_t& operator*=(const vec4_t& v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
		return *this;
	}

	vec4_t operator/(const vec4_t& v) const
	{
		return {x / v.x, y / v.y, z / v.z, w / v.w};
	}

	vec4_t& operator/=(const vec4_t& v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		w /= v.w;
		return *this;
	}

	vec4_t operator-()
	{
		return vec4_t(-x, -y, -z, -w);
	}

	bool operator<(const vec4_t& other)
	{
		return x < other.x && y < other.y && z < other.z;
	}

	bool operator>(const vec4_t& other)
	{
		return x > other.x && y > other.y && z > other.z && w > other.w;
	}

	T operator[](int32 index) const
	{
		return xyzw[index];
	}

	T& operator[](int32 index)
	{
		return xyzw[index];
	}

	operator vec2_t<T>() const
	{
		return {x, y};
	}

	operator vec3_t<T>() const
	{
		return {x, y, z};
	}
};

namespace Math
{
	template <typename T>
	static T cross(const vec2_t<T>& a, const vec2_t<T>& b)
	{
		return a.x * b.y - a.y * b.x;
	}

	/* Distance between two points in 3D space */
	template <typename T> static T distance(const vec2_t<T> v0, const vec2_t<T>& v1)
	{
		T a = Math::square(v1.x - v0.x);
		T b = Math::square(v1.y - v0.y);
		return std::sqrtf(a + b);
	}


	/* Distance between two points in 3D space */
	template <typename T>
	static T distance(const vec3_t<T> v0, const vec3_t<T>& v1)
	{
		T a = Math::square(v1.x - v0.x);
		T b = Math::square(v1.y - v0.y);
		T c = Math::square(v1.z - v0.z);
		return std::sqrtf(a + b + c);
	}

	template <typename T>
	static T dot(const vec4_t<T>& a, const vec4_t<T>& b)
	{
		T result = T(0);
		for (int32 index = 0; index < 4; index++)
		{
			result += a[index] * b[index];
		}
		return result;
	}

	template <typename T>
	static T area2D(const vec3_t<T>& v0, const vec3_t<T>& v1, const vec3_t<T>& v2)
	{
		T a = v0.x * (v1.y - v2.y);
		T b = v1.x * (v2.y - v0.y);
		T c = v2.x * (v0.y - v1.y);
		return std::abs((a + b + c) / T(2));
	}

	// Vector Sign
	template <typename T>
	static bool edgeSign(const vec2_t<T>& a, const vec2_t<T>& b, const vec2_t<T>& c)
	{
		T result = (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
		return Math::sign(result);
	}

	template <typename T>
	static float edgeValue(const vec2_t<T>& a, const vec2_t<T>& b, const vec2_t<T>& p)
	{
		return (b - a).cross(p - a);
	}

	/**
	 * Calculates the barycentric coordinates of a point P with respect to a triangle defined by vertices V0, V1, and V2.
	 *
	 * @param p The point to calculate the barycentric coordinates for.
	 * @param v0 The first vertex of the triangle.
	 * @param v1 The second vertex of the triangle.
	 * @param v2 The third vertex of the triangle.
	 * @param bary The output parameter that will store the calculated barycentric coordinates.
	 * @return True if the point is inside the triangle, false otherwise.
	 */
	template <typename T> 
	static bool isBarycentric(const vec2_t<T>& a, const vec2_t<T>& b, const vec2_t<T>& c, const vec2_t<T>& point)
	{
		if ((point.x == a.x && point.y == a.y) || (point.x == b.x && point.y == b.y) || (point.x == c.x && point.y == c.y))
		{
			return false;
		}

		float A = 0.5f * (-b.y * c.x + a.y * (-b.x + c.x) + a.x * (b.y - c.y) + b.x * c.y);
		float sign = A < 0.0f ? -1.0f : 1.0f;

		float s = (a.y * c.x - a.x * c.y + (c.y - a.y) * point.x + (a.x - c.x) * point.y) * sign;
		float t = (a.x * b.y - a.y * b.x + (a.y - b.y) * point.x + (b.x - a.x) * point.y) * sign;

		return s >= 0.0f && t >= 0.0f && (s + t) <= (2.0f * A * sign);
	}

	/**
	 * Calculates the barycentric coordinates of a point P with respect to a triangle defined by vertices V0, V1, and V2.
	 *
	 * @param p The point to calculate the barycentric coordinates for.
	 * @param v0 The first vertex of the triangle.
	 * @param v1 The second vertex of the triangle.
	 * @param v2 The third vertex of the triangle.
	 * @param bary The output parameter that will store the calculated barycentric coordinates.
	 * @return True if the point is inside the triangle, false otherwise.
	 */
	template <typename T>
	static bool getBarycentric(const vec3_t<T>& p,
	                           const vec3_t<T>& v0, const vec3_t<T>& v1, const vec3_t<T>& v2,
	                           vec3_t<T>& bary)
	{
		// Calculate the vectors representing the edges of the triangle
		const vec3_t<T> ba = v1 - v0;
		const vec3_t<T> ca = v2 - v0;
		const vec3_t<T> pa = p - v0;

		// Calculate the dot products
		const T d00 = ba.dot(ba);
		const T d01 = ba.dot(ca);
		const T d11 = ca.dot(ca);
		const T d20 = pa.dot(ba);
		const T d21 = pa.dot(ca);

		// Calculate the denominator of the formula
		const T denom = T(1) / (d00 * d11 - d01 * d01);

		// Calculate the barycentric coordinates
		const T v = (d11 * d20 - d01 * d21) * denom;
		const T w = (d00 * d21 - d01 * d20) * denom;
		const T u = T(1) - v - w;

		// Store the barycentric coordinates in the output parameter
		bary.x = u;
		bary.y = v;
		bary.z = w;

		// Check if the point is inside the triangle
		return bary.x >= T(0) && bary.y >= T(0) && bary.z >= T(0);
	}

	template <typename T> 
	static EWindingOrder getWindingOrder(const vec2_t<T>& v0, const vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		const float result = (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
		if (result == T(0))
		{
			return EWindingOrder::CoLinear;
		}
		return result > T(0) ? EWindingOrder::Clockwise : EWindingOrder::CounterClockwise;
	}

	template <typename T>
	static EWindingOrder getWindingOrder(const vec3_t<T>& v0, const vec3_t<T>& v1, const vec3_t<T>& v2)
	{
		const float result = (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
		if (result == T(0))
		{
			return EWindingOrder::CoLinear;
		}
		return result > T(0) ? EWindingOrder::Clockwise : EWindingOrder::CounterClockwise;
	}

	static constexpr float edgeFunction(const float x0, const float y0, const float x1, const float y1, const float x2,
	                                    const float y2)
	{
		return (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
	}

	template <typename T>
	static T edgeFunction(const vec3_t<T>& a, const vec3_t<T>& b, const vec3_t<T>& c)
	{
		return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	}

	template <typename T>
	static T getDepth(const vec3_t<T>& p, const vec3_t<T>& v0, const vec3_t<T>& v1, const vec3_t<T>& v2, T area)
	{
		// Calculate depth
		T w0 = Math::edgeFunction(v1, v2, p);
		T w1 = Math::edgeFunction(v2, v0, p);
		T w2 = Math::edgeFunction(v0, v1, p);

		w0 /= area;
		w1 /= area;
		w2 /= area;

		return v0.z * w0 + v1.z * w1 + v2.z * w2;
	}

	// // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
	template <typename T>
	static vec3_t<T> getSurfaceNormal(const vec3_t<T>& v0, const vec3_t<T>& v1, const vec3_t<T>& v2)
	{
		vec3_t<T> edge0  = v0 - v2;
		vec3_t<T> edge1  = v1 - v2;
		vec3_t<T> normal = edge0.cross(edge1);
		return normal.normalized();
	}
}; // namespace Math
