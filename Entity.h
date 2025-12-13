#pragma once
#include "GameObject.h"

class Entity : public GameObject {
protected:
    int health;
    int maxHealth;

public:
    Entity(int x, int y, int w, int h, int health, int color);

    virtual ~Entity() = default;

    virtual void takeDamage(int damage);
    virtual void heal(int amount);
    bool isAlive() const { return health > 0; }
    int getHealth() const { return health; }
    int getMaxHealth() const { return maxHealth; }

    // Базовый метод update (может быть пустым)
    virtual void update() {}
};