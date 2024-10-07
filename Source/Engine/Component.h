#pragma once

#include "Object.h"

class Component : public Object
{
public:
	Component()           = default;
	~Component() override = default;
	void update(float deltaTime) override {}
};
