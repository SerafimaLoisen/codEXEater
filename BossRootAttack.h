//#pragma once
//#include "BossAttack.h"
//#include <vector>
//#include <memory>
//
//class BossRoot;
//class GameEngine;
//
//// Структура для ожидающих корней
//struct PendingRoot {
//    int x, y;
//    bool fromTop;  // true = сверху вниз, false = снизу вверх
//    int timer;
//    bool isGrowing;
//};
//
//class BossRootAttack : public BossAttack {
//private:
//    int warningDuration;
//    int growDuration;
//    int rootDamage;
//    int rootColor;
//
//    std::vector<PendingRoot> pendingRoots;
//
//public:
//    BossRootAttack(Boss* boss, int cooldown, int warningDur, int growDur, int damage, int color);
//
//    void update(GameEngine& engine) override;  // Добавляем параметр!
//    void execute(GameEngine& engine) override;
//
//    void spawnRandomRoot(GameEngine& engine);
//    void spawnRoot(const PendingRoot& pending, GameEngine& engine);  // Добавляем GameEngine
//};