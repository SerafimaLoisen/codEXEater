#include "Boss.h"
#include "GameEngine.h"
#include "ConfigManager.h"
#include "GraphicsManager.h"
#include "BossBulletAttack.h"
#include "BossRootAttack.h"
#include <iostream>

Boss::Boss(int x, int y, int width, int height, int health,
    int phase2HP, int phase3HP,
    int bulletSpeed, int bulletColor, int bulletCooldown,
    int rootWarningDur, int rootGrowDur, int rootDamage, int rootCooldown, int rootColor)
    : Entity(x, y, width, height, health, 13),  // Пурпурный цвет
    currentPhase(Phase::PHASE1),
    phase2Threshold(phase2HP),
    phase3Threshold(phase3HP),
    bulletSpeed(bulletSpeed),
    bulletColor(bulletColor),
    bulletCooldown(bulletCooldown),
    rootWarningDuration(rootWarningDur),
    rootGrowDuration(rootGrowDur),
    rootDamage(rootDamage),
    rootCooldown(rootCooldown),
    rootColor(rootColor) {

    // Создаем атаки
    bulletAttack = std::make_unique<BossBulletAttack>(
        this, bulletCooldown, bulletSpeed, bulletColor
    );

    rootAttack = std::make_unique<BossRootAttack>(
        this, rootCooldown, rootWarningDuration,
        rootGrowDuration, rootDamage, rootColor
    );
}

void Boss::update(GameEngine& engine) {
    checkPhaseTransition();

    // Обновляем атаки
    bulletAttack->update(engine);
    rootAttack->update(engine);

    // Выполняем атаки в зависимости от фазы
    switch (currentPhase) {
    case Phase::PHASE1:
        if (bulletAttack->canExecute()) {
            bulletAttack->execute(engine);
            bulletAttack->resetCooldown();
        }
        break;

    case Phase::PHASE2:
        if (rootAttack->canExecute()) {
            rootAttack->execute(engine);
            rootAttack->resetCooldown();
        }
        break;

    case Phase::PHASE3:
        if (bulletAttack->canExecute()) {
            bulletAttack->execute(engine);
            bulletAttack->resetCooldown();
        }
        if (rootAttack->canExecute()) {
            rootAttack->execute(engine);
            rootAttack->resetCooldown();
        }
        break;
    }
}

void Boss::render() {
    static const auto& bossSprite = GraphicsManager::getGraphic("boss");
    GraphicsManager::renderAt(x, y, bossSprite, color);

    // Отображение здоровья босса (опционально)
    std::string healthBar;
    int barWidth = 20;
    int filled = (health * barWidth) / maxHealth;

    for (int i = 0; i < barWidth; i++) {
        if (i < filled) healthBar += "0";
        else healthBar += " ";
    }

    std::vector<std::string> healthBarGraphic = { "HP: " + healthBar + " " +
                                                 std::to_string(health) + "/" +
                                                 std::to_string(maxHealth) };

    GraphicsManager::renderAt(x, y - 2, healthBarGraphic, color);
}

void Boss::takeDamage(int damage) {
    health -= damage;
    if (health < 0) health = 0;

    // Мигание при получении урона
    static int damageFlashTimer = 0;
    damageFlashTimer = 5;
}

void Boss::checkPhaseTransition() {
    int healthPercent = (health * 100) / maxHealth;

    if (currentPhase == Phase::PHASE1 && healthPercent <= phase2Threshold) {
        currentPhase = Phase::PHASE2;
        std::cout << "\aБосс перешел во 2 фазу!" << std::endl;
    }
    else if (currentPhase == Phase::PHASE2 && healthPercent <= phase3Threshold) {
        currentPhase = Phase::PHASE3;
        std::cout << "\aБосс перешел в 3 фазу! Берегитесь!" << std::endl;
    }
}