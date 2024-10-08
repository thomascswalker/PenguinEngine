#pragma once
#include "Component.h"

#include "Engine/Mesh.h"

/** Component for displaying a static mesh. **/
class StaticMeshComponent : public Component
{
	/** Pointer to the mesh within g_meshes. **/
	Mesh* m_mesh = nullptr;

public:
	StaticMeshComponent()           = default;
	~StaticMeshComponent() override = default;

	void setMesh(Mesh* mesh)
	{
		m_mesh = mesh;
	}

	[[nodiscard]] Mesh* getMesh() const
	{
		return m_mesh;
	}
};
