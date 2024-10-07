#pragma once

#include "Mesh.h"
#include "Actor.h"

class StaticMeshActor : public Actor
{
	Mesh* m_mesh = nullptr;

public:
	StaticMeshActor() = default;

	~StaticMeshActor() override = default;

	void setMesh(Mesh* mesh)
	{
		m_mesh = mesh;
	}
};
