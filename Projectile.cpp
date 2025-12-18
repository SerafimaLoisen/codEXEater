#include "Projectile.h"

Projectile::Projectile(
    int x,
    int y,
    int width,
    int height,
    int color,
    bool enemy,
    int damage
)
    : GameObject(x, y, width, height, color),
    posX(static_cast<float>(x)),
    posY(static_cast<float>(y)),
    enemy(enemy),
    damage(damage)
{
}

void Projectile::update() {
    // --- двигаем float-позицию ---
    posX += velocityX;
    posY += velocityY;

    // --- синхронизируем int-координаты ---
    x = static_cast<int>(posX);
    y = static_cast<int>(posY);
}

void Projectile::setVelocity(float vx, float vy) {
    velocityX = vx;
    velocityY = vy;
}
