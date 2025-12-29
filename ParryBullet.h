#pragma once
#include "Projectile.h"

class ParryBullet : public Projectile {
public:
    ParryBullet(int x, int y, int direction);
    void render() override;
    void renderAt(int screenX, int screenY) const override;
};

/*
#pragma once
#include "Bullet.h"

class ParryBullet : public Bullet {
public:
    ParryBullet(int x, int y, float vx, float vy);

    void parry(float newSpeed);
    void render() override;
};
*/