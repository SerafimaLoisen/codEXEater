#include "BossBulletAttack.h"
#include "BossManager.h"
#include "Boss.h"
#include "FollowBullet.h"
#include "ConfigManager.h"

BossBulletAttack::BossBulletAttack(int cooldown, Player* player)
    : BossAttack(cooldown),
    player(player) {
}

void BossBulletAttack::execute(BossManager& manager, Boss& boss) {
    auto& cfg = ConfigManager::getInstance();

    auto bullet = std::make_unique<FollowBullet>(
        boss.getX(),
        boss.getY() + std::rand() % boss.getHeight(),
        player,
        cfg.getFollowBulletFollowDuration()
    );

    manager.spawnBossBullet(std::move(bullet));
}
