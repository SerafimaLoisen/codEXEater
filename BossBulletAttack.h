#pragma once

#include "BossAttack.h"
#include "EmitProjectilesComponent.h"

class Player;

class BossBulletAttack : public BossAttack {
public:
    BossBulletAttack(int cooldown, Player* player, std::vector<std::shared_ptr<Projectile>>&);

protected:
    void execute(BossManager& manager, Boss& boss) override;

private:
    Player* player;

	std::vector<int> direction = { -1, 0 };
	float projectileSpeed = 1;
	float projectileMaxTravelDistance = 40;
	int numberOfEmissionsInOneSequence = 4;
	int timeBetweenEmissionsInOneSequence = 2;
	int timeBetweenSequences = 4;
	bool emitTowardsTarget = false;
	bool limitDirectionToOneAxis = true;
	GameObject* projectilesTarget = nullptr;
	std::vector<std::shared_ptr<Projectile>>& projectilesArray;
	float projectileTypeFactor = 0;

	int emissionsTimer = 0;
	int sequencesTimer = 0;
	int emissionsCounter = 0;

	int shoot_point_x = 0;
	int shoot_point_y = 0;

	EmitProjectilesComponentConfig _config;

    //EmitProjectilesComponentConfig emitTowardsTarget;
    //std::shared_ptr<ComponentsBasedEntity> attackOnPlayerEnemy;
    //std::shared_ptr<EmitProjectilesComponent> emitProjectilesComponent;
};
