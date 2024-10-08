#pragma once

#include "Engine/Object.h"

/** Abstract base class for all components. **/
class Component : public Object
{
public:
	Component()           = default;
	~Component() override = default;
};
