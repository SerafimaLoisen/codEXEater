// Wall.cpp
#include "Wall.h"
#include "GraphicsManager.h"
#include <iostream>

Wall::Wall(int x, int y, int width, int height)
    : GameObject(x, y, width, height, 8) {  // 8 - цвет стен (серый)
}

void Wall::render() {
    std::vector<std::string> wallSprite;
    for (int i = 0; i < height; i++) {
        wallSprite.push_back(std::string(width, '#'));
    }
    GraphicsManager::renderAt(x, y, wallSprite, color);
}

void Wall::renderAt(int screenX, int screenY) const {
    std::vector<std::string> wallSprite;
    for (int i = 0; i < height; i++) {
        wallSprite.push_back(std::string(width, '#'));
    }
    GraphicsManager::renderAt(screenX, screenY, wallSprite, color);
}