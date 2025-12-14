#include "Projectile.h"
#include "GraphicsManager.h"

Projectile::Projectile(int x, int y, int width, int height, int speed, int direction, int color)
    : GameObject(x, y, width, height, color), speed(speed), direction(direction) {
    this->color = color;
    this->active = true;
}

void Projectile::update() {
    x += speed * direction;
}

void Projectile::renderAt(int screenX, int screenY) const {  // Добавьте const
    // Базовая реализация
    GraphicsManager::renderAt(screenX, screenY, { "*" }, color);
}