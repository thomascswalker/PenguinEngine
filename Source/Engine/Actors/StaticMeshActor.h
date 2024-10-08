#pragma once

#include "Actor.h"

#include "Engine/Object.h"
#include "Engine/ObjectManager.h"
#include "Engine/Components/StaticMeshComponent.h"

class StaticMeshActor : public Actor, public IRenderable
{
	StaticMeshComponent* m_staticMeshComponent = nullptr;

public:
	StaticMeshActor()
	{
		setSignature(ESignature::Renderable | ESignature::Tickable);
		m_staticMeshComponent = g_objectManager.createObject<StaticMeshComponent>();
	}

	~StaticMeshActor() override = default;

	[[nodiscard]] Mesh* getMesh() override
	{
		assert(m_staticMeshComponent != nullptr);
		return m_staticMeshComponent->getMesh();
	}

	void setMesh(Mesh* mesh) const
	{
		assert(m_staticMeshComponent != nullptr);
		m_staticMeshComponent->setMesh(mesh);
	}

	void update(float deltaTime) override
	{
		float rate = deltaTime * 0.1f;
		rotate(rate / 2, rate, 0);
	}

	transf getTransform() override
	{
		return m_transform;
	}
};
