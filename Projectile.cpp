#include "Projectile.h"
#include "GraphicsManager.h"

Projectile::Projectile(int x, int y, int width, int height, int speed, int direction, int color, bool _useFloatCoord, std::vector<float> _coord, std::vector<float> _velocity,
    bool enemy,
    int damage)
    : GameObject(x, y, width, height, color), speed(speed), direction(direction), useFloatCoord(_useFloatCoord), coord(_coord), velocity(_velocity),
    enemy(enemy),
    damage(damage) {
    this->color = color;
    this->active = true;
    coord[0] = x;
    coord[1] = y;
    initCoord = coord;
}

void Projectile::update() {

    if (!useFloatCoord) {
        x += speed * direction;
    }
    else {

        coord[0] -= velocity[0] * direction;
        coord[1] -= velocity[1] * direction;

        x = coord[0];
        y = coord[1];
    }

    if (abs(x - initCoord[0]) > maxTravelDistance || abs(y - initCoord[1]) > maxTravelDistance)
    {
        setActive(false);
    }
}

void Projectile::renderAt(int screenX, int screenY) const {
    GraphicsManager::renderAt(screenX, screenY, { "*" }, color);
}

/*

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
    // --- ������� float-������� ---
    posX += velocityX;
    posY += velocityY;

    // --- �������������� int-���������� ---
    x = static_cast<int>(posX);
    y = static_cast<int>(posY);
}

void Projectile::setVelocity(float vx, float vy) {
    velocityX = vx;
    velocityY = vy;
}

*/