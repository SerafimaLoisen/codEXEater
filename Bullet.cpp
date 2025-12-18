#include "Bullet.h"
#include "ConfigManager.h"
#include "GraphicsManager.h"

Bullet::Bullet(int x, int y, int direction)
    : Projectile(
        x, y, 1, 1,
        ConfigManager::getInstance().getBulletColor(),
        false,
        ConfigManager::getInstance().getBulletDamage()
    ) {
    float speed = static_cast<float>(
        ConfigManager::getInstance().getBulletSpeed()
        );

    setVelocity(speed * direction, 0.f);
}

void Bullet::update() {
    Projectile::update();
}

void Bullet::render() {
    GraphicsManager::renderAt(x, y, GraphicsManager::getGraphic("bullet"));
}
void Bullet::renderAt(int screenX, int screenY) const {
    GraphicsManager::renderAt(screenX, screenY, GraphicsManager::getGraphic("bullet"), color);
}
