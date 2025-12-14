#include "GameObject.h"
#include "GraphicsManager.h"

GameObject::GameObject(int x, int y, int width, int height, int color)
    : x(x), y(y), width(width), height(height), color(color), active(true) {
}

bool GameObject::checkCollision(const GameObject& other) const {
    return x < other.x + other.width &&
        x + width > other.x &&
        y < other.y + other.height &&
        y + height > other.y;
}

void GameObject::render() {
}

void GameObject::renderAt(int screenX, int screenY) const {
    for (int i = 0; i < height; i++) {
        GraphicsManager::renderAt(screenX, screenY + i, { std::string(width, '*') }, color);
    }
}