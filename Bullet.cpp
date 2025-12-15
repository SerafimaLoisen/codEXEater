#include "Bullet.h"
#include "ConfigManager.h"
#include "GraphicsManager.h"

Bullet::Bullet(int x, int y, int direction)
    : Projectile(x, y, 1, 1,
        ConfigManager::getInstance().getBulletSpeed(),
        direction, 7) {
}

void Bullet::render() {
    GraphicsManager::renderAt(x, y, GraphicsManager::getGraphic("bullet"), color);
}