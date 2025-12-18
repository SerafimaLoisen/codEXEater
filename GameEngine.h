#pragma once

#include <memory>
#include <vector>
#include <string>

class Player;
class Projectile;
class Platform;
class GameObject;
class BossManager;

class GameEngine {
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Projectile>> projectiles;
    std::vector<std::shared_ptr<Platform>> platforms;

    std::unique_ptr<BossManager> bossManager;

    bool gameRunning;
    int score;
    int screenWidth;
    int screenHeight;
    int parryBulletSpeed;
    int parryRange;
    std::string currentLevel;

public:
    GameEngine();
    ~GameEngine();

    void initialize(const std::string& levelName);
    void loadLevel(const std::string& levelName);
    void update();
    void render();

    void handlePlayerAttack();
    void handlePlayerCollisions();
    void handlePlayerWorldCollisions();
    void handlePlayerPlatformCollisions();
    void handleParry();
    bool isInParryRange(const GameObject& bullet) const;

    void createPlatformsFromUIFrame();
    void removePlatformsFromUIFrame();

    void addEnemyBullet(std::unique_ptr<Projectile> bullet);

    Player& getPlayer();
    const Player& getPlayer() const;

    bool isRunning() const;
    int getScore() const;
    void setGameRunning(bool running);

    std::vector<std::unique_ptr<Projectile>>& getProjectiles() {
        return projectiles;
    }
};
