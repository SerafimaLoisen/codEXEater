#pragma once
#include "ComponentsBasedEntity.h"
class IClonableComponent {
public:
	virtual IClonableComponent* clone(ComponentsBasedEntity& _target) = 0;

	virtual ~IClonableComponent() = default;
};

