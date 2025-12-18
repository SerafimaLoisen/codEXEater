#pragma once
#include "GameObject.h"

class Entity : public GameObject {
protected:
    int health;
    int maxHealth;
    int direction = 1;
public:
    Entity(int x, int y, int w, int h, int health, int color);

    virtual ~Entity() = default;

    virtual void takeDamage(int damage);
    bool isAlive() const { return health > 0; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }
    int getDirection() const { return direction; }
    void setDirection(int _direction) { direction = _direction; }

    // Базовый метод update (может быть пустым)
    virtual void update() {}

    virtual void renderAt(int screenX, int screenY) const override = 0;
};