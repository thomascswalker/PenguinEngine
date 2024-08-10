#pragma once

#include "Math/MathCommon.h"
#include "Math/Spherical.h"

enum class ECoordinateSpace
{
	Local,
	World
};

class Object
{
protected:
	transf m_transform;

	// Basis vectors
	vec3f m_forwardVector;
	vec3f m_rightVector;
	vec3f m_upVector;

public:
	virtual ~Object() = default;

	virtual void update(float deltaTime)
	{
	}

	void computeBasisVectors()
	{
		const FSphericalCoords tmp = FSphericalCoords::FromRotation(m_transform.Rotation);
		m_forwardVector = -tmp.ToCartesian().Normalized();
		// Negative because for some reason it defaults to the inverse
		m_rightVector = Math::Cross(vec3f::UpVector(), m_forwardVector).Normalized();
		m_upVector = Math::Cross(m_forwardVector, m_rightVector).Normalized();
	}

	// Getters
	transf getTransform() const { return m_transform; }
	vec3f getTranslation() const { return m_transform.Translation; }
	rotf getRotation() const { return m_transform.Rotation; }
	vec3f getScale() const { return m_transform.Scale; }

	// Setters
	void setTranslation(const vec3f& newTranslation) { m_transform.Translation = newTranslation; }

	void setRotation(const rotf& newRotation)
	{
		m_transform.Rotation = newRotation;
		computeBasisVectors();
	}

	void setScale(const vec3f& newScale) { m_transform.Scale = newScale; }

	// Manipulators
	void translate(const vec3f& delta)
	{
		m_transform.Translation += delta;
	}

	void rotate(const float pitch, const float yaw, const float roll)
	{
		m_transform.Rotation += rotf(pitch, yaw, roll);
		m_transform.Rotation.Normalize();
		computeBasisVectors();
	}

	// Axes

	vec3f getForwardVector() const
	{
		return m_forwardVector;
	}

	vec3f getBackwardVector() const
	{
		return -m_forwardVector;
	}

	vec3f getRightVector() const
	{
		return m_rightVector;
	}

	vec3f getLeftVector() const
	{
		return -m_rightVector;
	}

	vec3f getUpVector() const
	{
		return m_upVector;
	}

	vec3f getDownVector() const
	{
		return -m_upVector;
	}
};
