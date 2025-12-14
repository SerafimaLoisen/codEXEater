#pragma once
#include "Projectile.h"

class Bullet : public Projectile {
public:
    Bullet(int x, int y, int direction);
    void render() override;
    void renderAt(int screenX, int screenY) const override;
};