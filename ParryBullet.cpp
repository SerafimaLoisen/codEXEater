#include "ParryBullet.h"
#include "ConfigManager.h"
#include "GraphicsManager.h"
#include <cmath>

ParryBullet::ParryBullet(int x, int y, float vx, float vy)
    : Bullet(x, y, 1)  // direction тут не важен
{
    enemy = true;
    damage = ConfigManager::getInstance().getParryBulletDamage();
    color = ConfigManager::getInstance().getParryBulletColor();

    setVelocity(vx, vy);
}

void ParryBullet::parry(float newSpeed) {
    // отражаем по X и ускоряем
    float dirX = (velocityX >= 0.f) ? 1.f : -1.f;
    setVelocity(-dirX * newSpeed, velocityY * 0.5f);

    enemy = false; // теперь это "дружественная" пуля
}

void ParryBullet::render() {
    GraphicsManager::renderAt(x, y,
        GraphicsManager::getGraphic("parry_bullet"),
        color
    );
}
