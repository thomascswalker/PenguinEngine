#pragma once

#include "Core/Macros.h"
#include "Engine/Components/Component.h"
#include "Engine/Object.h"
#include "Engine/ObjectManager.h"
#include "Math/MathCommon.h"

/** Represents an object in the scene which can tick and has a transform. **/
class Actor : public Object, public ITickable
{
	GENERATE_SUPER(Object)

protected:
	std::vector<Component*> m_components;

	transf m_transform;

	// Basis vectors
	vec3f m_forwardVector;
	vec3f m_rightVector;
	vec3f m_upVector;

public:
	Actor();

	~Actor() override = default;
	void update(float deltaTime) override = 0;
	void computeBasisVectors();

	/** Getters **/

	[[nodiscard]] transf getTransform() const;
	[[nodiscard]] vec3f getTranslation() const;
	[[nodiscard]] rotf getRotation() const;
	[[nodiscard]] vec3f getScale() const;

	/** Setters **/

	void setTranslation(const vec3f& newTranslation);
	void setRotation(const rotf& newRotation);
	void setScale(const vec3f& newScale);

	/** Manipulators **/

	void translate(const vec3f& delta);
	void rotate(float pitch, float yaw, float roll);

	/** Axes **/

	[[nodiscard]] vec3f getForwardVector() const;
	[[nodiscard]] vec3f getBackwardVector() const;
	[[nodiscard]] vec3f getRightVector() const;
	[[nodiscard]] vec3f getLeftVector() const;
	[[nodiscard]] vec3f getUpVector() const;
	[[nodiscard]] vec3f getDownVector() const;

	/** Components **/

	template <typename T>
	Component* addComponent()
	{
		Component* comp = g_objectManager.createObject<T>();
		if (comp)
		{
			m_components.emplace_back(comp);
		}
		return comp;
	}
};
