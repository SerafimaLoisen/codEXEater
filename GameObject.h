#pragma once
#include <string>

class GameObject {
protected:
    int x, y;
    int width, height;
    int color;
    bool active;

public:
    GameObject(int x, int y, int width, int height, int color);
    virtual ~GameObject() = default;

    virtual void update() = 0;
    virtual void render() = 0;

    // Базовые геттеры
    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getColor() const { return color; }

    bool isActive() const { return active; }

    // Базовые сеттеры
    void setX(int newX) { x = newX; }
    void setY(int newY) { y = newY; }
    void setActive(bool isActive) { active = isActive; }
    void setColor(int newColor) { color = newColor; }

    // Проверка столкновений
    bool checkCollision(const GameObject& other) const;
};