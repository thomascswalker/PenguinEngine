#pragma once

#include <map>

#include "Component.h"
#include "ObjectManager.h"

class ComponentManager
{
	std::vector<Component*> m_components{};

public:
	ComponentManager() = default;

	void addComponent(Component* component)
	{
		m_components.emplace_back(component);
	}
};
