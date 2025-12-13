#pragma once
#include "Player.h"
#include "Projectile.h" 
#include "Platform.h"
#include <vector>
#include <memory>
#include "Boss.h"
#include "BossRoot.h"

class GameEngine {
private:
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    std::vector<std::shared_ptr<Platform>> platforms;
    std::unique_ptr<Boss> boss;
    std::vector<std::unique_ptr<BossRoot>> bossRoots;
    int bulletSpawnTimer;
    bool gameRunning;
    int score;

    void handlePlayerAttack();

    // Кэшированные конфигурационные параметры
    int screenWidth;
    int screenHeight;
    int parryBulletSpeed;
    int parryRange;

    // Уровень игры
    std::string currentLevel;
    bool bossMode;

public:
    GameEngine();

    // Инициализация с выбором уровня
    void initialize(const std::string& levelName = "tutorial");

    // Основные игровые методы
    void update();
    void render();

    // Обработка коллизий
    void handlePlayerCollisions();
    void handlePlayerWorldCollisions();
    void handlePlayerPlatformCollisions();

    // Работа с пулями
    void spawnBullet();
    void checkCollisions();
    void handleParry();

    void spawnBoss();
    void updateBoss();
    void renderBoss();
    void checkBossCollisions();

    void addEnemyBullet(std::unique_ptr<Bullet> bullet);
    void addBossRoot(std::unique_ptr<BossRoot> root);

    Boss* getBoss() { return boss.get(); }

    // Работа с уровнями
    void loadLevel(const std::string& levelName);
    void loadGraphicsForLevel();
    void createPlatformsFromUIFrame();
    void removePlatformsFromUIFrame();

    // Утилиты
    bool isInParryRange(const GameObject& bullet) const;

    // Геттеры
    Player& getPlayer();
    const Player& getPlayer() const;
    bool isRunning() const;
    int getScore() const;
    std::string getCurrentLevel() const { return currentLevel; }
    bool isBossMode() const { return bossMode; }

    // Сеттеры
    void setGameRunning(bool running);
    void setBossMode(bool mode) { bossMode = mode; }

    int getScreenHeight() const { return screenHeight; }
    int getScreenWidth() const { return screenWidth; }
    Player* getPlayerPtr() { return player.get(); }

};