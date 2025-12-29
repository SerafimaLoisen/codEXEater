#pragma once
#include "GrowDirection.h"
#include <string>
#include <vector>

class BossRoot {
public:
    BossRoot(
        int x, int y,
        GrowDirection dir,
        int maxLength,
        int damage,
        int color,
        int collisionCooldownMax
    );

    void update();   // рост и обновление кулдауна
    void renderAt(int screenX, int screenY);   // отрисовка

    // ===== Геттеры для коллизий =====
    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;

    int getDamage() const { return damage; }
    int getHealth() const { return health; }
    void takeDamage(int dmg) { health -= dmg; }

    // ===== Кулдаун для столкновения =====
    bool canCollide();
    void resetCollisionCooldown();

private:
    int startX;
    int startY;
    GrowDirection direction;


    std::vector<std::string> graphic;


    int length;        // текущая длина
    int maxLength;     // максимальная длина

    int damage;
    int color;
    int health;

    bool isGrown;

    int collisionCooldown = 0;
    int collisionCooldownMax = 10;
};
