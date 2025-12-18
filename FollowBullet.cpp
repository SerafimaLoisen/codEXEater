#include "FollowBullet.h"
#include "Player.h"
#include "ConfigManager.h"

FollowBullet::FollowBullet(
    int x,
    int y,
    Player* target,
    int homingDuration
)
    : Bullet(x, y, -1),
    target(target),
    homingTimer(homingDuration)
{
    auto& cfg = ConfigManager::getInstance();

    enemy = true;
    damage = cfg.getFollowBulletDamage();
    color = cfg.getFollowBulletColor();

    setVelocity(-cfg.getFollowBulletSpeed(), 0.f);
}

void FollowBullet::update() {
    if (homingTimer > 0 && target) {

        float targetX = target->getX() + target->getWidth() * 0.5f;
        float targetY = target->getY() + target->getHeight() * 0.5f;

        float dx = targetX - posX;
        float dy = targetY - posY;

        float length = std::sqrt(dx * dx + dy * dy);
        if (length > 0.001f) {
            dx /= length;
            dy /= length;

            float speed = std::sqrt(
                velocityX * velocityX + velocityY * velocityY
            );

            velocityX = dx * speed;
            velocityY = dy * speed;
        }

        --homingTimer;
    }

    Projectile::update();
}
