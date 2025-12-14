#include "Platform.h"
#include "GraphicsManager.h"
#include <iostream>

Platform::Platform(int x, int y, int width)
    : GameObject(x, y, width, 1, 7) { 
}

void Platform::render() {
    std::string platformLine = std::string(width, '=');
    GraphicsManager::renderAt(x, y, { platformLine });
}

void Platform::renderAt(int screenX, int screenY) const {  // Добавьте const
    std::string platformLine = std::string(width, '=');
    GraphicsManager::renderAt(screenX, screenY, { platformLine }, color);
}