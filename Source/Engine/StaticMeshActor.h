#pragma once

#include "Mesh.h"
#include "Actor.h"

class StaticMeshActor : public Actor, public IRenderable
{
	Mesh* m_mesh = nullptr;

public:
	StaticMeshActor()
	{
		setSignature(ESignature::Renderable | ESignature::Tickable);
	}

	~StaticMeshActor() override = default;

	[[nodiscard]] Mesh* getMesh() override
	{
		return m_mesh;
	}

	void setMesh(Mesh* mesh)
	{
		m_mesh = mesh;
	}

	void update(float deltaTime) override {}

	transf getTransform() override
	{
		return m_transform;
	}
};
