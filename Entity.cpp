#include "Entity.h"
#include "Logger.h"

Entity::Entity(int x, int y, int w, int h, int health, int color)
    : GameObject(x, y, w, h, color), health(health), maxHealth(health) {
}

void Entity::takeDamage(int damage) {
    health -= damage;
    if (health < 0) health = 0;
    Logger::Log("Player took " + std::to_string(damage) + " damage. Health: " +
        std::to_string(health) + "/" + std::to_string(maxHealth));
}
