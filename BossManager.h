#pragma once
#include <memory>
#include <vector>
#include "GrowDirection.h"
#include "Camera.h"

class GameEngine;
class Boss;
class BossAttack;
class BossRoot;
class Player;
class Projectile;

class BossManager {
public:
    explicit BossManager(GameEngine* engine);
    ~BossManager();

    void update();
    void render(Camera& camera);

    // ===== корни =====
    void spawnRoot(int x, int y, GrowDirection dir, int maxLength, int damage, int color);

    // ===== пули =====
    //void spawnBossBullet(std::unique_ptr<Projectile> bullet);

    // ===== столкновения =====
    void checkPlayerCollisions(Player& player);
    void checkPlayerBulletCollisions(std::shared_ptr<Projectile>& bullet);

    // ===== утилиты =====
    void renderWarning(int x, int y, int color);
    void restartFight();

    Boss& getBoss();
    GameEngine* getEngine();

private:
    GameEngine* engine;
    std::unique_ptr<Boss> boss;

    std::vector<std::unique_ptr<BossAttack>> bulletAttacks;
    std::vector<std::unique_ptr<BossAttack>> rootAttacks;
    std::vector<std::unique_ptr<BossRoot>> roots;

    bool checkAABB(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh);
};
