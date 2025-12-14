#include "ParryBullet.h"
#include "ConfigManager.h"
#include "GraphicsManager.h"

ParryBullet::ParryBullet(int x, int y, int direction)
    : Projectile(x, y, 1, 1,
        ConfigManager::getInstance().getParryBulletSpeed(),
        direction, 13) {
}

void ParryBullet::render() {
    GraphicsManager::renderAt(x, y, GraphicsManager::getGraphic("parry_bullet"), 13);
}

void ParryBullet::renderAt(int screenX, int screenY) const {  // Добавьте const
    GraphicsManager::renderAt(screenX, screenY, GraphicsManager::getGraphic("parry_bullet"), 13);
}