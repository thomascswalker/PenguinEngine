#pragma once

/** Represents an object in the scene which can tick, but has no transform. **/
class Object
{
public:
	Object() {}
	virtual ~Object() = default;
	virtual void update(float deltaTime) = 0;
};
