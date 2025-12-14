#pragma once
#include "Player.h"
#include "Projectile.h" 
#include "Platform.h"
#include <vector>
#include <memory>
#include "Boss.h"
#include "BossRoot.h"
#include "Camera.h"
#include "SidePlatform.h"

class GameEngine {
private:
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    std::vector<std::shared_ptr<Platform>> platforms;
    std::vector<std::shared_ptr<SidePlatform>> sidePlatforms;
    //std::unique_ptr<Boss> boss;
    //std::vector<std::unique_ptr<BossRoot>> bossRoots;
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

    void updateCamera();  // Новый метод для обновления камеры
    void renderWithCamera();  // Новый метод для рендеринга с камерой

    // Измените сигнатуры методов рендеринга
    void renderGameObject(const GameObject& obj) const;
    void renderProjectile(const Projectile& projectile) const;
    void renderUIFrameWithCamera();

    void handlePlayerSidePlatformCollisions();

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

    /*void spawnBoss();
    void updateBoss();
    void renderBoss();
    void checkBossCollisions();*/

    void addEnemyBullet(std::unique_ptr<Bullet> bullet);
    //void addBossRoot(std::unique_ptr<BossRoot> root);

    //Boss* getBoss() { return boss.get(); }

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

    void normalizeUIFrame();
    int getVisualLength(const std::string& str) const;
    std::string getVisibleSubstring(const std::string& str, int startPos, int maxWidth) const;
    void switchLevel(const std::string& levelName);
    int getVisualPosition(const std::string& str, int bytePos) const;
    bool findPlayerSpawn(int& spawnX, int& spawnY);
    void removeSpawnPointFromUIFrame(int spawnX, int spawnY);
};