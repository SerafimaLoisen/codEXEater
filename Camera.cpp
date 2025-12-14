#include "Camera.h"
#include "Logger.h"
#include <algorithm>
#include <iostream>

Camera::Camera(int viewportW, int viewportH, int worldW, int worldH)
    : viewportWidth(viewportW), viewportHeight(viewportH),
    worldWidth(worldW), worldHeight(worldH),
    x(0), y(0) {}

void Camera::setPosition(int newX, int newY) {
    int maxX = std::max(0, worldWidth - viewportWidth);
    int maxY = std::max(0, worldHeight - viewportHeight);

    x = std::clamp(newX, 0, maxX);
    y = std::clamp(newY, 0, maxY);

    Logger::Log("Camera position set: (" + std::to_string(x) + ", " +
        std::to_string(y) + ") maxX=" + std::to_string(maxX));
}

void Camera::centerOn(int targetX, int targetY) {
    int newX = targetX - viewportWidth / 2;
    int newY = targetY - viewportHeight / 2;

    Logger::Log("Camera::centerOn: target(" + std::to_string(targetX) + "," + std::to_string(targetY) +
        ") -> new(" + std::to_string(newX) + "," + std::to_string(newY) + ")");

    setPosition(newX, newY);
}

int Camera::worldToScreenX(int worldX) const {
    return worldX - x;
}

int Camera::worldToScreenY(int worldY) const {
    return worldY - y;
}

bool Camera::isInViewport(int worldX, int worldY, int width, int height) const {
    return (worldX + width > x && worldX < x + viewportWidth &&
        worldY + height > y && worldY < y + viewportHeight);
}

void Camera::updateViewportSize(int newWidth, int newHeight) {
    viewportWidth = newWidth;
    viewportHeight = newHeight;
    setPosition(x, y);
}