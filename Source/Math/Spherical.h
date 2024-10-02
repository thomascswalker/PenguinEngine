#pragma once

#include "Vector.h"
#include "Rotator.h"

// https://github.com/mrdoob/three.js/blob/cb24e42a65172ec475ff23a4abe520b724076a24/examples/jsm/controls/OrbitControls.js
struct sphericalf
{
	float phi    = 0.0f; // yaw, horizontal angle in radians
	float theta  = 0.0f; // pitch, vertical angle in radians
	float radius = 5.0f;

	void makeSafe(const float threshold = EPSILON)
	{
		phi = std::max(threshold, std::min(g_pi - threshold, phi));
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
		s.makeSafe();
		return s;
	}

	void rotateRight(const float angle)
	{
		theta -= angle;
	}

	void rotateUp(const float angle)
	{
		phi -= angle;
		makeSafe();
	}
};
