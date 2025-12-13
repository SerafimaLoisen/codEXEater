#pragma once
#include "Entity.h"
#include "BossAttack.h"
#include <memory>
#include <vector>

class Boss : public Entity {
private:
    enum class Phase { PHASE1, PHASE2, PHASE3 };
    Phase currentPhase;
    int phase2Threshold;
    int phase3Threshold;

    std::unique_ptr<BossAttack> bulletAttack;
    std::unique_ptr<BossAttack> rootAttack;

    // Настройки
    int bulletSpeed;
    int bulletColor;
    int bulletCooldown;
    int rootWarningDuration;
    int rootGrowDuration;
    int rootDamage;
    int rootCooldown;
    int rootColor;

public:
    Boss(int x, int y, int width, int height, int health,
        int phase2HP, int phase3HP,
        int bulletSpeed, int bulletColor, int bulletCooldown,
        int rootWarningDur, int rootGrowDur, int rootDamage, int rootCooldown, int rootColor);

    // УБИРАЕМ override здесь, т.к. Entity::update() может не существовать
    void update(GameEngine& engine);

    // Entity уже имеет render(), делаем override
    void render() override;

    // Entity уже имеет takeDamage(), делаем override
    void takeDamage(int damage) override;

    Phase getCurrentPhase() const { return currentPhase; }
    void checkPhaseTransition();

    // Геттеры
    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};