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

	virtual void update(float deltaTime) {}

	void computeBasisVectors()
	{
		const sphericalf tmp = sphericalf::fromRotation(m_transform.rotation);
		m_forwardVector      = -tmp.toCartesian().normalized();
		// Negative because for some reason it defaults to the inverse
		m_rightVector = vec3f::upVector().cross(m_forwardVector).normalized();
		m_upVector    = m_forwardVector.cross(m_rightVector).normalized();
	}

	// Getters
	transf getTransform() const
	{
		return m_transform;
	}

	vec3f getTranslation() const
	{
		return m_transform.translation;
	}

	rotf getRotation() const
	{
		return m_transform.rotation;
	}

	vec3f getScale() const
	{
		return m_transform.scale;
	}

	// Setters
	void setTranslation(const vec3f& newTranslation)
	{
		m_transform.translation = newTranslation;
	}

	void setRotation(const rotf& newRotation)
	{
		m_transform.rotation = newRotation;
		computeBasisVectors();
	}

	void setScale(const vec3f& newScale)
	{
		m_transform.scale = newScale;
	}

	// Manipulators
	void translate(const vec3f& delta)
	{
		m_transform.translation += delta;
	}

	void rotate(const float pitch, const float yaw, const float roll)
	{
		m_transform.rotation += rotf(pitch, yaw, roll);
		m_transform.rotation.normalize();
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
