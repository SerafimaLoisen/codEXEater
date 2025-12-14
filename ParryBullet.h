#pragma once
#include "Projectile.h"

class ParryBullet : public Projectile {
public:
    ParryBullet(int x, int y, int direction);
    void render() override;
    void renderAt(int screenX, int screenY) const override;
};