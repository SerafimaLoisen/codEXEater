#pragma once
#include "IClonableComponent.h"

class ComponentsBasedEntity;

class EntityComponent : public IClonableComponent
{
public:
	EntityComponent(ComponentsBasedEntity* _target);
	virtual void Process() = 0;
	virtual EntityComponent* clone(ComponentsBasedEntity& _target) = 0;
protected:
	ComponentsBasedEntity* target;
};

