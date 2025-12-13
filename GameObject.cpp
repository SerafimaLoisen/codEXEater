#include "GameObject.h"

GameObject::GameObject(int x, int y, int width, int height, int color)
    : x(x), y(y), width(width), height(height), color(color), active(true) {
}

bool GameObject::checkCollision(const GameObject& other) const {
    return x < other.x + other.width &&
        x + width > other.x &&
        y < other.y + other.height &&
        y + height > other.y;
}