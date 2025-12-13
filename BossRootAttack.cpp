#include "BossRootAttack.h"
#include "Boss.h"
#include "GameEngine.h"
#include "BossRoot.h"
#include <cstdlib>
#include <ctime>

// Конструктор
BossRootAttack::BossRootAttack(Boss* boss, int cooldown, int warningDur,
    int growDur, int damage, int color)
    : BossAttack(boss, cooldown),
    warningDuration(warningDur),
    growDuration(growDur),
    rootDamage(damage),
    rootColor(color) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

// Update с параметром GameEngine
void BossRootAttack::update(GameEngine& engine) {
    BossAttack::update(); // Вызываем базовый update

    // Обновляем предупреждения
    for (int i = pendingRoots.size() - 1; i >= 0; --i) {
        pendingRoots[i].timer--;

        if (pendingRoots[i].timer <= 0) {
            if (!pendingRoots[i].isGrowing) {
                // Начинаем рост
                pendingRoots[i].isGrowing = true;
                pendingRoots[i].timer = growDuration;
            }
            else {
                // Создаем корень
                spawnRoot(pendingRoots[i], engine);
                pendingRoots.erase(pendingRoots.begin() + i);
            }
        }
    }
}

// Execute
void BossRootAttack::execute(GameEngine& engine) {
    spawnRandomRoot(engine);
}

// Создание случайного корня
void BossRootAttack::spawnRandomRoot(GameEngine& engine) {
    if (!boss) return;

    int screenHeight = engine.getScreenHeight();
    int bossX = boss->getX();

    // Случайные координаты для корня
    int rootX = bossX - (std::rand() % 40 + 10);
    int rootY = 0;
    bool fromTop = (std::rand() % 2 == 0);

    if (fromTop) {
        rootY = 5 + (std::rand() % 5);
    }
    else {
        rootY = screenHeight - 10 - (std::rand() % 5);
    }

    // Создаем предупреждение
    PendingRoot pending;
    pending.x = rootX;
    pending.y = rootY;
    pending.fromTop = fromTop;
    pending.timer = warningDuration;
    pending.isGrowing = false;

    pendingRoots.push_back(pending);
}

// Создание корня
void BossRootAttack::spawnRoot(const PendingRoot& pending, GameEngine& engine) {
    // Создаем корень с помощью make_unique
    auto root = std::make_unique<BossRoot>(
        pending.x, pending.y,
        !pending.fromTop,  // Если сверху вниз, то растет вниз
        8,                 // Максимальная высота
        rootDamage,
        rootColor
    );

    // Добавляем в GameEngine
    engine.addBossRoot(std::move(root));
}