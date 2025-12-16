#pragma once
#include "EntityComponent.h"
#include <vector>
#include <memory>

class GameObject;

class TakeDamageOnOverlapComponent : public EntityComponent
{
private:
	std::vector<std::shared_ptr<GameObject>>& candidatesForOverlap;
	int damageAmount = 1;
	int initColor;
	int damageColor = 5;
	int damageDuration = 1;
	int damageTimer = 0;
public:
	TakeDamageOnOverlapComponent(ComponentsBasedEntity* _target, std::vector<std::shared_ptr<GameObject>>& _candidatesForOverlap, int _damageAmount=1, int _damageColor = 10);
	void Process() override;
	TakeDamageOnOverlapComponent* clone(ComponentsBasedEntity& _target) override;
};

