#pragma once
#include "Projectile.h"

class Bullet : public Projectile {
public:
    Bullet(int x, int y, int direction);

    void update() override;
    void render() override;
};
