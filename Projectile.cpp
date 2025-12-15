#include "Projectile.h"
#include "GraphicsManager.h"

Projectile::Projectile(int x, int y, int width, int height, int speed, int direction, int color, bool _useFloatCoord, std::vector<float> _coord, std::vector<float> _velocity)
    : GameObject(x, y, width, height, color), speed(speed), direction(direction), useFloatCoord(_useFloatCoord), coord(_coord), velocity(_velocity) {
    this->color = color;
    this->active = true;
    coord[0] = x;
    coord[1] = y;
}

void Projectile::update() {
    if (!useFloatCoord) {
        x += speed * direction;
    }
    else {
        
        coord[0] += velocity[0];
        coord[1] += velocity[1];

        x = coord[0];
        y = coord[1];
    }
}

void Projectile::renderAt(int screenX, int screenY) const {
    GraphicsManager::renderAt(screenX, screenY, { "*" }, color);
}