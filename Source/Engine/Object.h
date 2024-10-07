#pragma once

#include <array>
#include <queue>

#include "Mesh.h"

#include "Core/Bitmask.h"

using ObjectId = uint32;

enum class ESignature : uint8
{
	None       = 0,
	Tickable   = 1,
	Renderable = 2,
};

DEFINE_BITMASK_OPERATORS(ESignature)

class IRenderable
{
public:
	virtual Mesh* getMesh() = 0;
	virtual transf getTransform() = 0;
};

class ITickable
{
public:
	virtual void update(float deltaTime) = 0;
};

/** Represents an object in the scene which cannot tick and has no transform. **/
class Object
{
protected:
	/** Unique ID for this object. */
	ObjectId m_objectId = 0;
	/** Signature of core engine features. */
	ESignature m_signature = ESignature::None;

public:
	Object()          = default;
	virtual ~Object() = default;

	[[nodiscard]] ObjectId getObjectId() const
	{
		return m_objectId;
	}

	void setObjectId(const ObjectId objectId)
	{
		m_objectId = objectId;
	}

	[[nodiscard]] ESignature getSignature() const
	{
		return m_signature;
	}

	[[nodiscard]] bool hasSignature(const ESignature signature) const
	{
		return (m_signature & signature) == signature;
	}

	void setSignature(const ESignature signature)
	{
		m_signature = signature;
	}
};
