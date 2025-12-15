#pragma once
#include "Entity.h"
#include "IClonable.h"

class EntityComponent : public IClonableComponent
{
public:
	EntityComponent(Entity* _target);
	virtual void Process() = 0;
	virtual EntityComponent* clone(Entity& _target) = 0;
protected:
	Entity* target;
};

