#pragma once
#include "GameObject.h"

class Projectile : public GameObject {
protected:
    // --- ѕЋј¬Ќјя позици€ ---
    float posX;
    float posY;

    // --- скорость ---
    float velocityX = 0.f;
    float velocityY = 0.f;

    bool enemy;
    int damage;

public:
    Projectile(
        int x,
        int y,
        int width,
        int height,
        int color,
        bool enemy,
        int damage
    );

    virtual ~Projectile() = default;

    virtual void update();

    bool isEnemy() const { return enemy; }
    int getDamage() const { return damage; }

    void setVelocity(float vx, float vy);
};
