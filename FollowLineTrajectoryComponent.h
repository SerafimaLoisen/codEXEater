#pragma once
#include "EntityComponent.h"
#include <vector>
#include "ComponentConfig.h"

class Entity;

enum class Axis { X=0, Y=1 };

struct FollowLineTrajectoryComponentConfig : public ComponentConfig
{

public:

	FollowLineTrajectoryComponentConfig(bool justWalkBetweenSides = false, 
		int _speed = 1, std::vector<int> _offset = { 2, 2 }, int _startDirection = 1, 
		Axis _axis = Axis::X, int _restDuration = 4, int startDelay = 0);

	FollowLineTrajectoryComponentConfig() = default;

	bool justWalkBetweenSides = false;
	std::vector<int> offset = { 2, 2 };
	int speed = 1;
	int direction = 1;
	Axis axis = Axis::X;

	int restDuration = 4;
	int startDelay = 0;
};

class FollowLineTrajectoryComponent : public EntityComponent
{
public:
	FollowLineTrajectoryComponent(ComponentsBasedEntity* _target, FollowLineTrajectoryComponentConfig _config);
	FollowLineTrajectoryComponent() = default;
	void Process() override;
	FollowLineTrajectoryComponent* clone(ComponentsBasedEntity& _target) override;
private:
	// almost useless mode
	bool justWalkBetweenSides = false;
	// { TO LEFT OFFSET, TO RIGHT OFFSET } FOR X AXIS
	// { TO TOP OFFSET, TO BOTTOM OFFSET } FOR Y AXIS
	std::vector<int> offset = {2, 2};
	int speed = 1;
	int direction = 1;
	Axis axis = Axis::X;
	
	int restDuration = 4;

	std::vector<int> initCoord;
	int restingTimer = 0;
	FollowLineTrajectoryComponentConfig config;
};

