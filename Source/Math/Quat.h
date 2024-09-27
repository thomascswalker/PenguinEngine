#pragma once

template <typename T>
struct quat_t
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

	quat_t() : x(0), y(0), z(0), w(0) {}

	quat_t(T inX, T inY, T inZ, T inW) : x(inX), y(inY), z(inZ), w(inW) {}

	quat_t(vec3_t<T> axis, T angle)
	{
		const T halfAngle = 0.5f * angle;
		T s, c;
		Math::sinCos(&s, &c, halfAngle);

		x = s * axis.x;
		y = s * axis.y;
		z = s * axis.z;
		w = c;
	}

	quat_t(vec3_t<T> from, vec3_t<T> to)
	{
		T r = from.dot(to) + T(1);
		if (r < EPSILON)
		{
			r = 0;
			if (std::abs(from.x) > std::abs(from.z))
			{
				x = -from.y;
				y = from.x;
				z = 0;
				w = r;
			}
			else
			{
				x = 0;
				y = -from.z;
				z = from.y;
				w = r;
			}
		}
		else
		{
			x = from.y * to.z - from.z * to.y;
			y = from.z * to.x - from.x * to.z;
			z = from.x * to.y - from.y * to.x;
			w = r;
		}
		normalize();
	}

	quat_t(const mat4_t<T>& m)
	{
		//const MeReal *const t = (MeReal *) tm;
		T s;

		// Check diagonal (trace)
		const T tr = m.m[0][0] + m.m[1][1] + m.m[2][2];

		if (tr > 0.0f)
		{
			T invS  = Math::invSqrt(tr + 1.f);
			this->w = 0.5f * (1.f / invS);
			s       = 0.5f * invS;

			this->x = ((m.m[1][2] - m.m[2][1]) * s);
			this->y = ((m.m[2][0] - m.m[0][2]) * s);
			this->z = ((m.m[0][1] - m.m[1][0]) * s);
		}
		else
		{
			// diagonal is negative
			int32 i = 0;

			if (m.m[1][1] > m.m[0][0])
				i = 1;

			if (m.m[2][2] > m.m[i][i])
				i = 2;

			constexpr int32 nxt[3] = {1, 2, 0};
			const int32 j          = nxt[i];
			const int32 k          = nxt[j];

			s = m.m[i][i] - m.m[j][j] - m.m[k][k] + 1.0f;

			T invS = Math::invSqrt(s);

			T qt[4];
			qt[i] = 0.5f * (1.f / invS);

			s = 0.5f * invS;

			qt[3] = (m.m[j][k] - m.m[k][j]) * s;
			qt[j] = (m.m[i][j] + m.m[j][i]) * s;
			qt[k] = (m.m[i][k] + m.m[k][i]) * s;

			this->x = qt[0];
			this->y = qt[1];
			this->z = qt[2];
			this->w = qt[3];
		}
	}

	// From Euler angles
	quat_t(const float pitch, const float yaw, const float roll)
	{
		T cy = cosf(yaw * 0.5f);
		T sy = sinf(yaw * 0.5f);
		T cp = cosf(pitch * 0.5f);
		T sp = sinf(pitch * 0.5f);
		T cr = cosf(roll * 0.5f);
		T sr = sinf(roll * 0.5f);

		w = cr * cp * cy + sr * sp * sy;
		x = sr * cp * cy - cr * sp * sy;
		y = cr * sp * cy + sr * cp * sy;
		z = cr * cp * sy - sr * sp * cy;
	}

	quat_t operator*(const quat_t& other) const
	{
		T tempW = w * other.w - x * other.x - y * other.y - z * other.z;
		T tempX = w * other.x + x * other.w + y * other.z - z * other.y;
		T tempY = w * other.y - x * other.y + y * other.w + z * other.x;
		T tempZ = w * other.z + x * other.z - y * other.x + z * other.w;

		return {tempX, tempY, tempZ, tempW};
	}

	vec4_t<T> operator*(const vec4_t<T>& other)
	{
		quat_t quatVector{other.x, other.y, other.z, other.w};
		quat_t inverse = getConjugate();
		quat_t temp    = *this;
		quat_t out     = temp * quatVector * inverse;
		return {out.x, out.y, out.z, out.w};
	}

	quat_t getConjugate() const
	{
		return {-x, -y, -z, w};
	}

	// Alias of GetConjugate
	quat_t getInverse() const
	{
		return {-x, -y, -z, w};
	}

	vec3_t<T> rotateVector(vec3_t<T> v)
	{
		const vec3_t<T> q(x, y, z);
		const vec3_t<T> tt     = q.cross(v) * 2.0f;
		const vec3_t<T> result = v + (tt * w) + q.cross(tt);
		return result;
	}

	T length()
	{
		return std::sqrtf(x * x + y * y + z * z + w * w);
	}

	void normalize()
	{
		T l = length();
		if (l == 0)
		{
			x = T(0);
			y = T(0);
			z = T(0);
			w = T(1);
		}
		else
		{
			l = T(1) / l;
			x *= l;
			y *= l;
			z *= l;
			w *= l;
		}
	}
};

//
// template <>
// rotf quatf::Rotator();
// template <>
// rotd quatd::Rotator();
