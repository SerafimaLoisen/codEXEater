#include "SidePlatform.h"
#include "GraphicsManager.h"
#include <iostream>

SidePlatform::SidePlatform(int x, int y, int height)
    : GameObject(x, y, 1, height, 8) {
}

void SidePlatform::update() {
}

void SidePlatform::render() {
    std::vector<std::string> platformLines;
    for (int i = 0; i < height; i++) {
        platformLines.push_back("#");
    }
    GraphicsManager::renderAt(x, y, platformLines, color);
}

void SidePlatform::renderAt(int screenX, int screenY) const {
    std::vector<std::string> platformLines;
    for (int i = 0; i < height; i++) {
        platformLines.push_back("#");
    }
    GraphicsManager::renderAt(screenX, screenY, platformLines, color);
}