#include "FollowLineTrajectoryComponent.h"
#include "ComponentsBasedEntity.h"

FollowLineTrajectoryComponentConfig::FollowLineTrajectoryComponentConfig(
	bool _justWalkBetweenSides, int _speed, std::vector<int> _offset, int _startDirection,
	Axis _axis, int _restDuration, int _startDelay) :
	offset(_offset), speed(_speed), direction(_startDirection),
	axis(_axis), restDuration(_restDuration), startDelay(_startDelay), justWalkBetweenSides(_justWalkBetweenSides)
{
}

FollowLineTrajectoryComponent::FollowLineTrajectoryComponent(ComponentsBasedEntity* _target, FollowLineTrajectoryComponentConfig _config) : EntityComponent(_target), config(_config) {
	initCoord = std::vector<int>{ _target->getX(), _target->getY() };
	
	justWalkBetweenSides = _config.justWalkBetweenSides;
	offset = _config.offset;
	speed = _config.speed;
	axis = _config.axis;
	restDuration = _config.restDuration;
	restingTimer = _config.startDelay;

	if (offset[0] == 0) {
		direction = 1;
	}
	else if (offset[1] == 0) {
		direction = -1;
	}
	else {
		direction = _config.direction;
	}
	_target->setDirection(direction);
}

void FollowLineTrajectoryComponent::Process() {

	if (restingTimer <= 0) 
	{
		int coord = axis == Axis::X ? target->getX() : target->getY();
		coord += speed * target->getDirection() * (axis == Axis::Y ? -1 : 1);
		if (axis == Axis::X) target->setX(coord);
		else target->setY(coord);
		
		if (!(target->GetIsGravityEnabled() && justWalkBetweenSides)) 
		{
			// DIRECTION CHANGE
			if (coord >= initCoord[static_cast<int>(axis)] + offset[1] || coord <= initCoord[static_cast<int>(axis)] - offset[0]) {
				//direction *= -1;
				target->setDirection(-1 * target->getDirection());
				restingTimer = restDuration;
			}
		}
		target->SetHorizontalVelocity(speed);
	}
	else if (restingTimer > 0) {
		restingTimer--;

		target->SetHorizontalVelocity(0);
	}

}

FollowLineTrajectoryComponent* FollowLineTrajectoryComponent::clone(ComponentsBasedEntity& _target) {
	FollowLineTrajectoryComponent* clone = new FollowLineTrajectoryComponent(&_target, config);
	return clone;
}
