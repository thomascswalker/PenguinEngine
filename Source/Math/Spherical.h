#pragma once

#include "Vector.h"
#include "Rotator.h"

// https://github.com/mrdoob/three.js/blob/cb24e42a65172ec475ff23a4abe520b724076a24/examples/jsm/controls/OrbitControls.js
struct sphericalf
{
	float phi    = 0.0f; // yaw, horizontal angle in radians
	float theta  = 0.0f; // pitch, vertical angle in radians
	float radius = 5.0f;

	sphericalf() = default;

	sphericalf(const vec3f& v)
	{
		float x = v.x;
		float y = v.y;
		float z = v.z;
		radius  = std::sqrtf(x * x + y * y + z * z);
		if (radius == 0.0f)
		{
			theta = 0.0f;
			phi   = 0.0f;
		}
		else
		{
			theta = std::atan2f(x, z);
			phi   = std::acosf(std::clamp(y / radius, -1.0f, 1.0f));
		}
	}

	sphericalf(const float x, const float y, const float z)
	{
		radius = std::sqrtf(x * x + y * y + z * z);
		if (radius == 0.0f)
		{
			theta = 0.0f;
			phi   = 0.0f;
		}
		else
		{
			theta = std::atan2f(x, z);
			phi   = std::acosf(std::clamp(y / radius, -1.0f, 1.0f));
		}
	}

	void makeSafe(const float threshold = SMALL_NUMBER)
	{
		phi = std::clamp(phi, threshold, g_pi - threshold);
	}

	static sphericalf fromCartesian(const float x, const float y, const float z)
	{
		sphericalf s;
		s.radius = std::sqrtf(x * x + y * y + z * z);
		if (s.radius == 0.0f)
		{
			s.theta = 0.0f;
			s.phi   = 0.0f;
		}
		else
		{
			s.theta = std::atan2f(x, z);
			s.phi   = std::acosf(std::clamp(y / s.radius, -1.0f, 1.0f));
		}
		return s;
	}

	vec3f toCartesian() const
	{
		const float sinPhiRadius = std::sinf(phi) * radius;
		return vec3f{
			sinPhiRadius * std::sinf(theta),
			std::cosf(phi) * radius,
			sinPhiRadius * std::cosf(theta)
		};
	}

	static sphericalf fromRotation(const rotf& rot)
	{
		sphericalf s;
		s.theta = Math::degreesToRadians(rot.pitch);
		s.phi   = Math::degreesToRadians(rot.yaw);
		//s.makeSafe();
		return s;
	}

	void rotateRight(const float angle)
	{
		theta += angle;
	}

	void rotateUp(const float angle)
	{
		phi += angle;
		//makeSafe();
	}

	void zero(const float threshold = 0.01f)
	{
		theta = 0.0f;
		phi = 0.0f;
	}
};
