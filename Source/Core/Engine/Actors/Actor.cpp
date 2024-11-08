#include "Actor.h"

#include "Math/Spherical.h"

Actor::Actor()
{
	setSignature(ESignature::Tickable);
}

void Actor::computeBasisVectors()
{
	const sphericalf tmp = sphericalf::fromRotation(m_transform.rotation);

	// Negative because for some reason it defaults to the inverse
	m_forwardVector = -tmp.toCartesian().normalized();

	m_rightVector = vec3f::upVector().cross(m_forwardVector).normalized();
	m_upVector    = m_forwardVector.cross(m_rightVector).normalized();
}

transf Actor::getTransform() const
{
	return m_transform;
}

vec3f Actor::getTranslation() const
{
	return m_transform.translation;
}

rotf Actor::getRotation() const
{
	return m_transform.rotation;
}

vec3f Actor::getScale() const
{
	return m_transform.scale;
}

void Actor::setTranslation(const vec3f& newTranslation)
{
	m_transform.translation = newTranslation;
}

void Actor::setRotation(const rotf& newRotation)
{
	m_transform.rotation = newRotation;
	computeBasisVectors();
}

void Actor::setScale(const vec3f& newScale)
{
	m_transform.scale = newScale;
}

void Actor::translate(const vec3f& delta)
{
	m_transform.translation += delta;
}

void Actor::rotate(const float pitch, const float yaw, const float roll)
{
	m_transform.rotation += rotf(pitch, yaw, roll);
	m_transform.rotation.normalize();
	computeBasisVectors();
}

vec3f Actor::getForwardVector() const
{
	return m_forwardVector;
}

vec3f Actor::getBackwardVector() const
{
	return -m_forwardVector;
}

vec3f Actor::getRightVector() const
{
	return m_rightVector;
}

vec3f Actor::getLeftVector() const
{
	return -m_rightVector;
}

vec3f Actor::getUpVector() const
{
	return m_upVector;
}

vec3f Actor::getDownVector() const
{
	return -m_upVector;
}
