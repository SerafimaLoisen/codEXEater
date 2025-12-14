#include "Camera.h"
#include "Logger.h"
#include <algorithm>
#include <iostream>

Camera::Camera(int viewportW, int viewportH, int worldW, int worldH)
    : viewportWidth(viewportW), viewportHeight(viewportH),
    worldWidth(worldW), worldHeight(worldH),
    x(0), y(0) {

    // Логируем для отладки
    Logger::Log("Camera created with world: " +
        std::to_string(worldW) + "x" + std::to_string(worldH));
}

void Camera::setPosition(int newX, int newY) {
    // Рассчитываем границы с учетом размера viewport
    int maxX = std::max(0, worldWidth - viewportWidth);
    int maxY = std::max(0, worldHeight - viewportHeight);

    // Ограничиваем позицию
    x = std::clamp(newX, 0, maxX);
    y = std::clamp(newY, 0, maxY);

    Logger::Log("Camera position set: (" + std::to_string(x) + ", " +
        std::to_string(y) + ") maxX=" + std::to_string(maxX));
}

void Camera::centerOn(int targetX, int targetY) {
    // Центрируем камеру на цели, учитывая границы
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
    // После изменения размера нужно скорректировать позицию
    setPosition(x, y);
}