#pragma once
#include "Entity.h"

class IClonableEntity
{
public:
	virtual IClonableEntity* clone(int _x, int _y) = 0;

	virtual ~IClonableEntity() = default;
};

class IClonableComponent {
public:
	virtual IClonableComponent* clone(Entity& _target) = 0;

	virtual ~IClonableComponent() = default;
};

