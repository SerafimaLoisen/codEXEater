#include "FollowLineTrajectoryComponent.h"
#include "Entity.h"

FollowLineTrajectoryComponentConfig::FollowLineTrajectoryComponentConfig(std::vector<int> _offset,
	int _speed, int _direction, Axis _axis, int _restDuration) : offset(_offset), speed(_speed), direction(_direction), axis(_axis), restDuration(_restDuration) {

}

FollowLineTrajectoryComponent::FollowLineTrajectoryComponent(Entity* _target, FollowLineTrajectoryComponentConfig _config) : EntityComponent(_target), config(_config) {
	initCoord = std::vector<int>{ _target->getX(), _target->getY() };
	
	offset = _config.offset;
	speed = _config.speed;
	direction = _config.direction;
	axis = _config.axis;
	restDuration = _config.restDuration;
}

void FollowLineTrajectoryComponent::Process() {

	if (restingTimer <= 0) {

		int coord = axis == Axis::X ? target->getX() : target->getY();
		coord += speed * direction;
		if (axis == Axis::X) target->setX(coord);
		else target->setY(coord);

		// DIRECTION CHANGE
		if (coord >= initCoord[static_cast<int>(axis)] + offset[1] || coord <= initCoord[static_cast<int>(axis)] - offset[0]) {
			direction *= -1;
			restingTimer = restDuration;
		}
	}
	else if (restingTimer > 0) {
		restingTimer--;
	}

}

FollowLineTrajectoryComponent* FollowLineTrajectoryComponent::clone(Entity& _target) {
	FollowLineTrajectoryComponent* clone = new FollowLineTrajectoryComponent(&_target, config);
	return clone;
}