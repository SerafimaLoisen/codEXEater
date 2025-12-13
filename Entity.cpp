#include "Entity.h"

Entity::Entity(int x, int y, int w, int h, int health, int color)
    : GameObject(x, y, w, h, color), health(health), maxHealth(health) {
}

void Entity::takeDamage(int damage) {
    health -= damage;
    if (health < 0) health = 0;
}

void Entity::heal(int amount) {
    health += amount;
    if (health > maxHealth) health = maxHealth;
}