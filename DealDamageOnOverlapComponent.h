#pragma once
#include "EntityComponent.h"
#include "ComponentConfig.h"

class Player;

struct DealDamageOnOverlapComponentConfig : ComponentConfig {

	DealDamageOnOverlapComponentConfig(int _contactDamageAmount=1, int _contactDamageCooldownDuration=2, bool _ignorePlayerDodgingOnContact=true);

	int contactDamageAmount = 1;
	int contactDamageCooldownDuration = 2;
	bool ignorePlayerDodgingOnContact = true;
};

class DealDamageOnOverlapComponent : public EntityComponent
{
public:
	DealDamageOnOverlapComponent(Entity* _target, Player& _player, DealDamageOnOverlapComponentConfig _config);
	void Process() override;
	DealDamageOnOverlapComponent* clone(Entity& _target) override;
private:
	int contactDamageAmount;
	int contactDamageCooldownDuration;
	int contactDamageCooldownTimer = 0;
	bool ignorePlayerDodgingOnContact;

	Player& player;
	DealDamageOnOverlapComponentConfig config;
};

