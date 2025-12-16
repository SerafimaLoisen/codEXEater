#include "DealDamageOnOverlapComponent.h"
#include "Player.h"
#include "ComponentsBasedEntity.h"

DealDamageOnOverlapComponentConfig::DealDamageOnOverlapComponentConfig(
	int _contactDamageAmount, int _contactDamageCooldownDuration, 
	bool _ignorePlayerDodgingOnContact) : 
	contactDamageAmount(_contactDamageAmount), 
	contactDamageCooldownDuration(_contactDamageCooldownDuration),
	ignorePlayerDodgingOnContact(_ignorePlayerDodgingOnContact){
}

DealDamageOnOverlapComponent::DealDamageOnOverlapComponent(
	ComponentsBasedEntity* _target,
	Player& _player, 
	DealDamageOnOverlapComponentConfig _config) : 
	EntityComponent(_target), player(_player), config(_config) {
	contactDamageAmount = config.contactDamageAmount;
	contactDamageCooldownDuration = config.contactDamageCooldownDuration;
	ignorePlayerDodgingOnContact = config.ignorePlayerDodgingOnContact;
}

void DealDamageOnOverlapComponent::Process() {

	if (target->checkCollision(player) && contactDamageCooldownTimer <= 0) {

		if (ignorePlayerDodgingOnContact || !player.getIsDodging())
		{
			player.takeDamage(contactDamageAmount);
			contactDamageCooldownTimer = contactDamageCooldownDuration;
		}
	}
	else if (contactDamageCooldownTimer > 0) {
		contactDamageCooldownTimer--;
	}
}

DealDamageOnOverlapComponent* DealDamageOnOverlapComponent::clone(ComponentsBasedEntity& _target) {
	DealDamageOnOverlapComponent* clone = new DealDamageOnOverlapComponent(&_target, player, config);
	return clone;
}
