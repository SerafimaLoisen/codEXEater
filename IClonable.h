#pragma once

class IClonableEntity
{
public:
	virtual IClonableEntity* clone(int _x, int _y) = 0;

	virtual ~IClonableEntity() = default;
};

