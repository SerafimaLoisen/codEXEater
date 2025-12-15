#pragma once
#include "EntityComponent.h"
#include <vector>
#include "ComponentConfig.h"

class Entity;

enum class Axis { X=0, Y=1 };

struct FollowLineTrajectoryComponentConfig : public ComponentConfig
{

public:

	FollowLineTrajectoryComponentConfig(std::vector<int> _offset, 
		int _speed, int _direction, Axis _axis, int _restDuration);

	FollowLineTrajectoryComponentConfig() = default;

	std::vector<int> offset = { 2, 2 };
	int speed = 1;
	int direction = 1;
	Axis axis = Axis::X;

	int restDuration = 4;
};

class FollowLineTrajectoryComponent : public EntityComponent
{
public:
	FollowLineTrajectoryComponent(Entity* _target, FollowLineTrajectoryComponentConfig _config);
	FollowLineTrajectoryComponent() = default;
	void Process() override;
	FollowLineTrajectoryComponent* clone(Entity& _target) override;
private:
	std::vector<int> offset = {2, 2};
	int speed = 1;
	int direction = 1;
	Axis axis = Axis::X;
	
	int restDuration = 4;

	std::vector<int> initCoord;
	int restingTimer = 0;
	FollowLineTrajectoryComponentConfig config;
};

