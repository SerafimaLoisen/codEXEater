#include "BossBulletAttack.h"
#include "BossManager.h"
#include "Boss.h"
#include "FollowBullet.h"
#include "ConfigManager.h"
#include "GameEngine.h"
#include "ParryBullet.h"

BossBulletAttack::BossBulletAttack(int cooldown, Player* player, std::vector<std::shared_ptr<Projectile>>& projectiles)
    : BossAttack(cooldown), 
    player(player), projectilesArray(projectiles){

    //EmitProjectilesComponentConfig emitTowardsTarget = EmitProjectilesComponentConfig({ -1, 0 }, 2, 40, 3, 3, 18, 0, true, false, 0.5f);

    //std::shared_ptr<ComponentsBasedEntity> attackOnPlayerEnemy = std::make_shared<ComponentsBasedEntity>(20, 5, 5, 3, 3, true);

    //emitProjectilesComponent = std::make_shared<EmitProjectilesComponent>(
    //    attackOnPlayerEnemy.get(), // осторожно с сырым указателем!
    //    emitTowardsTarget,
    //    projectiles,
    //    player);



    _config = EmitProjectilesComponentConfig({ -1, 0 }, 2, 70, 3, 3, 18, 0, true, false, 0.5f);
    //attackOnPlayerEnemy = std::make_shared<ComponentsBasedEntity>(20, 5, 5, 3, 3, true);
    //attackOnPlayerEnemy->AddComponent(*new EmitProjectilesComponent(attackOnPlayerEnemy.get(), emitTowardsTarget, projectiles, player));


    projectilesArray = projectiles;
    projectilesTarget = player;

    direction = _config.direction;
    projectileSpeed = _config.projectileSpeed;
    projectileMaxTravelDistance = _config.projectileMaxTravelDistance;
    numberOfEmissionsInOneSequence = _config.numberOfEmissionsInOneSequence;
    timeBetweenEmissionsInOneSequence = _config.timeBetweenEmissionsInOneSequence;
    timeBetweenSequences = _config.timeBetweenSequences;
    sequencesTimer = _config.startDelay;
    emitTowardsTarget = _config.emitTowardsTarget;
    limitDirectionToOneAxis = _config.limitDirectionToOneAxis;
    projectileTypeFactor = _config.projectileTypeFactor;


    shoot_point_x = 60;
    shoot_point_y = 10;
}

void BossBulletAttack::execute(BossManager& manager, Boss& boss) {
    //auto& cfg = ConfigManager::getInstance();

    //auto bullet = std::make_unique<FollowBullet>(
    //    boss.getX(),
    //    boss.getY() + std::rand() % boss.getHeight(),
    //    player,
    //    cfg.getFollowBulletFollowDuration()
    //);

    //auto bullet = std::make_unique<Projectile>(
    //    boss.getX(),
    //    boss.getY() + std::rand() % boss.getHeight(),
    //    player,
    //    cfg.getFollowBulletFollowDuration()
    //);

    //manager.spawnBossBullet(std::move(bullet));

    //if (emitProjectilesComponent) {
    //    emitProjectilesComponent->Process();
    //}

    //attackOnPlayerEnemy->update();

    shoot_point_x = boss.getX();
    shoot_point_y = boss.getY() + std::rand() % boss.getHeight();



    std::vector<float> velocity;

    std::vector<float> dir = { static_cast<float>((projectilesTarget->getX() + projectilesTarget->getWidth() / 2) - shoot_point_x), static_cast<float>((projectilesTarget->getY() + projectilesTarget->getHeight() / 2) - shoot_point_y) };

    float length = sqrt(dir[0] * dir[0] + dir[1] * dir[1]);

    dir[0] /= length;
    dir[1] /= length;

    velocity = { dir[0] * projectileSpeed, dir[1] * projectileSpeed };

    if (limitDirectionToOneAxis) {
        if (abs(dir[0]) > abs(dir[1])) {
            velocity = { (dir[0] > 0 ? static_cast<float>(projectileSpeed) : -projectileSpeed), 0 };
        }
        else velocity = { 0, (dir[1] > 0 ? static_cast<float>(projectileSpeed) : -projectileSpeed) };
    }



    float rnd = (std::rand() % 100) / 100.0;
    bool regularType = rnd >= projectileTypeFactor;

    std::shared_ptr<Projectile> projectile;

    std::vector<int> coord = { shoot_point_x , shoot_point_y };

    if (regularType) projectile = std::make_shared<Bullet>(coord[0], coord[1], -1);
    else projectile = std::make_shared<ParryBullet>(coord[0], coord[1], -1);

    projectile->setUseFloatCoord(true);
    projectile->setVelocity(velocity);
    projectile->setMaxTravelDistance(projectileMaxTravelDistance);
    projectile->setColor(12);

    projectilesArray.push_back(projectile);
}