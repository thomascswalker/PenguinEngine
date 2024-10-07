#pragma once

#include "Object.h"

#include "Math/MathCommon.h"
#include "Math/Spherical.h"

enum class ECoordinateSpace
{
	Local,
	World
};

/** Represents an object in the scene which both has a transform and can tick. **/
class Actor : public Object
{
protected:
	transf m_transform;

	// Basis vectors
	vec3f m_forwardVector;
	vec3f m_rightVector;
	vec3f m_upVector;

public:
	Actor() {}
	~Actor() override = default;
	void update(float deltaTime) override {}

	void computeBasisVectors()
	{
		const sphericalf tmp = sphericalf::fromRotation(m_transform.rotation);
		m_forwardVector      = -tmp.toCartesian().normalized();
		// Negative because for some reason it defaults to the inverse
		m_rightVector = vec3f::upVector().cross(m_forwardVector).normalized();
		m_upVector    = m_forwardVector.cross(m_rightVector).normalized();
	}

	// Getters
	[[nodiscard]] transf getTransform() const
	{
		return m_transform;
	}

	[[nodiscard]] vec3f getTranslation() const
	{
		return m_transform.translation;
	}

	[[nodiscard]] rotf getRotation() const
	{
		return m_transform.rotation;
	}

	[[nodiscard]] vec3f getScale() const
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

	[[nodiscard]] vec3f getForwardVector() const
	{
		return m_forwardVector;
	}

	[[nodiscard]] vec3f getBackwardVector() const
	{
		return -m_forwardVector;
	}

	[[nodiscard]] vec3f getRightVector() const
	{
		return m_rightVector;
	}

	[[nodiscard]] vec3f getLeftVector() const
	{
		return -m_rightVector;
	}

	[[nodiscard]] vec3f getUpVector() const
	{
		return m_upVector;
	}

	[[nodiscard]] vec3f getDownVector() const
	{
		return -m_upVector;
	}
};
