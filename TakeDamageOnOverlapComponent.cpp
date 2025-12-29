#include "TakeDamageOnOverlapComponent.h"
#include "ComponentsBasedEntity.h"

TakeDamageOnOverlapComponent::TakeDamageOnOverlapComponent(
	ComponentsBasedEntity* _target,
	std::vector<std::shared_ptr<GameObject>>& _candidatesForOverlap,
	int _damageAmount, int _damageColor) : 
	EntityComponent(_target),
	candidatesForOverlap(_candidatesForOverlap),
	damageAmount(_damageAmount),
	damageColor(_damageColor)
{
	initColor = _target->getColor();
}

void TakeDamageOnOverlapComponent::Process() {

	if (damageTimer <= 0) {
		target->setColor(initColor);
		for (auto& other : candidatesForOverlap) {
			if (target->checkCollision(*other)) {
				target->takeDamage(damageAmount);
				target->setColor(damageColor);
				damageTimer = damageDuration;
			}
		}
	}
	else damageTimer--;
}

TakeDamageOnOverlapComponent* TakeDamageOnOverlapComponent::clone(ComponentsBasedEntity& _target)
{
	TakeDamageOnOverlapComponent* clone = new TakeDamageOnOverlapComponent(&_target, candidatesForOverlap, damageAmount);
	return clone;
}
