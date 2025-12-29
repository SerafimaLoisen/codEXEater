#pragma once
#include "Player.h"
#include "Projectile.h" 
#include "Platform.h"
#include <vector>
#include <memory>
#include <map>
#include <iostream>
#include <windows.h>
#include "Boss.h"
#include "BossRoot.h"
#include "Camera.h"
#include "SidePlatform.h"
#include "IClonable.h"
#include "ConfigManager.h"
#include "BossManager.h"

class ComponentsBasedEntity;

class Checkpoint {
public:
    Checkpoint(int x, int y) : x(x), y(y), active(false) {}

    int getX() const { return x; }
    int getY() const { return y; }
    bool isActive() const { return active; }
    void setActive(bool active) { this->active = active; }

    void render() const {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD coord;
        coord.X = x;
        coord.Y = y;
        SetConsoleCursorPosition(hConsole, coord);

        if (active) {
            SetConsoleTextAttribute(hConsole, ConfigManager::getInstance().getCheckpointActiveColor());
        }
        else {
            SetConsoleTextAttribute(hConsole, ConfigManager::getInstance().getCheckpointInactiveColor());
        }

        std::cout << ConfigManager::getInstance().getCheckpointSymbol();
        SetConsoleTextAttribute(hConsole, 7);
    }

private:
    int x, y;
    bool active;
};

class GameEngine {
private:
    std::unique_ptr<Camera> camera;
    std::shared_ptr<Player> player;
    std::vector<std::shared_ptr<Projectile>> projectiles;
    std::vector<std::shared_ptr<Platform>> platforms;
    std::vector<std::shared_ptr<SidePlatform>> sidePlatforms;
    std::vector<std::shared_ptr<Checkpoint>> checkpoints; // Вектор чекпоинтов
    std::shared_ptr<Checkpoint> currentCheckpoint; // Текущий активный чекпоинт
    int levelStartX, levelStartY; // Координаты начала уровня

    std::unique_ptr<Boss> boss;
    std::unique_ptr<BossManager> bossManager;
    std::vector<std::unique_ptr<BossRoot>> bossRoots;

    int bulletSpawnTimer;
    bool gameRunning;
    int score;

    void handlePlayerAttack();

    // РљСЌС€РёСЂРѕРІР°РЅРЅС‹Рµ РєРѕРЅС„РёРіСѓСЂР°С†РёРѕРЅРЅС‹Рµ РїР°СЂР°РјРµС‚СЂС‹
    int screenWidth;
    int screenHeight;
    int parryBulletSpeed;
    int parryRange;

    bool showDeathScreen;

    // РЈСЂРѕРІРµРЅСЊ РёРіСЂС‹
    std::string currentLevel;
    bool bossMode;


    void updateCamera();
    void renderWithCamera();


    // РР·РјРµРЅРёС‚Рµ СЃРёРіРЅР°С‚СѓСЂС‹ РјРµС‚РѕРґРѕРІ СЂРµРЅРґРµСЂРёРЅРіР°
    void renderGameObject(const GameObject& obj) const;
    void renderProjectile(const Projectile& projectile) const;
    void renderUIFrameWithCamera();

    void createCheckpointsFromUIFrame(); // Создание чекпоинтов из UIFrame
    void activateCheckpoint(std::shared_ptr<Checkpoint> checkpoint); // Активация чекпоинта
    void respawnAtLevelStart(); // Возрождение в начале уровня
    bool findLevelStart(int& startX, int& startY); // Поиск точки начала уровня

    void handlePlayerSidePlatformCollisions();

    int ourClamp(int value, int min, int max);
	
    static GameEngine* instance;

#pragma region ENEMIES

private:

    typedef std::map<std::string, std::shared_ptr<ComponentsBasedEntity>> HostileEntitiesPrefabs;
    typedef std::shared_ptr<ComponentsBasedEntity> CBE;
    
    HostileEntitiesPrefabs GlobalHostileEntitiesPrefabs;
    HostileEntitiesPrefabs LevelBasedHostileEntitiesPrefabs;
    std::vector<std::shared_ptr<ComponentsBasedEntity>> hostileEntitiesToProcess;
    std::vector<char> reservedSymbols = { '=', '#', '^', '>', '<', 'v', 'p', 's' };
    std::vector<char> spikes = { '^', 'v', '>', '<' };
    std::vector<std::shared_ptr<GameObject>> playerProjectiles;

    void CreateHostileEntitiesFromLevelMap(std::shared_ptr<HostileEntitiesPrefabs> levelBasedHostileEntitiesPrefabs, std::string levelMapName = "UIFrame");
    void DefineGlobalHostileEntitiesPrefabs();
    std::shared_ptr<GameEngine::HostileEntitiesPrefabs>  DefineLevelBasedHostileEntitiesPrefabs(int levelIndex = 0);
    std::string GetEntityIDFromUIFrame(int x, int y, std::vector<std::string>& uiFrame);
    CBE GetPrefabFromID(std::string ID, HostileEntitiesPrefabs& curLevelHostileEntitiesPrefabs);
#pragma endregion


public:
    GameEngine();

    // РРЅРёС†РёР°Р»РёР·Р°С†РёСЏ СЃ РІС‹Р±РѕСЂРѕРј СѓСЂРѕРІРЅСЏ
    void initialize(const std::string& levelName = "tutorial");

    // РћСЃРЅРѕРІРЅС‹Рµ РёРіСЂРѕРІС‹Рµ РјРµС‚РѕРґС‹
    // Доступ к активному экземпляру GameEngine
    static GameEngine* getInstance();

    // Основные игровые методы
    void update();
    void render();

    // РћР±СЂР°Р±РѕС‚РєР° РєРѕР»Р»РёР·РёР№
    void handlePlayerCollisions();
    void handlePlayerWorldCollisions();
    void handlePlayerPlatformCollisions();

    // Р Р°Р±РѕС‚Р° СЃ РїСѓР»СЏРјРё
    void spawnBullet();
    void checkCollisions();
    void handleParry();


    std::vector<std::shared_ptr<Projectile>>& getProjectiles() { return projectiles; };
        


    void setDeathScreenFlag(bool flag);
    bool getDeathScreenFlag();


    //void spawnBoss();
    //void updateBoss();
    //void renderBoss();
    //void checkBossCollisions();

    //void addEnemyBullet(std::unique_ptr<Bullet> bullet);
    //void addBossRoot(std::unique_ptr<BossRoot> root);

    //Boss* getBoss() { return boss.get(); }

    // Р Р°Р±РѕС‚Р° СЃ СѓСЂРѕРІРЅСЏРјРё
    void loadLevel(const std::string& levelName);
    void loadGraphicsForLevel();
    void createPlatformsFromUIFrame();
    void removePlatformsFromUIFrame();

    // РЈС‚РёР»РёС‚С‹
    bool isInParryRange(const GameObject& bullet) const;

    // Р“РµС‚С‚РµСЂС‹
    Player& getPlayer();
    const Player& getPlayer() const;
    bool isRunning() const;
    int getScore() const;
    std::string getCurrentLevel() const { return currentLevel; }
    bool isBossMode() const { return bossMode; }

    // РЎРµС‚С‚РµСЂС‹
    void setGameRunning(bool running);
    void setBossMode(bool mode) { bossMode = mode; }

    int getScreenHeight() const { return screenHeight; }
    int getScreenWidth() const { return screenWidth; }
    Player* getPlayerPtr() { return player.get(); }

    void normalizeUIFrame();
    int getVisualLength(const std::string& str) const;
    std::string getVisibleSubstring(const std::string& str, int startVisualPos, int maxVisualWidth) const;
    void switchLevel(const std::string& levelName);
    int getVisualPosition(const std::string& str, int bytePos) const;
    bool findPlayerSpawn(int& spawnX, int& spawnY);
    void removeSpawnPointFromUIFrame(int spawnX, int spawnY);
    void respawnAtCheckpoint(); // Возрождение на чекпоинте
    std::shared_ptr<Checkpoint> getCurrentCheckpoint() const { return currentCheckpoint; }
    void checkWinCondition();
    void finishBossFight();
    void removeWinSymbolsFromUIFrame();



    bool endGame;
};



// !!! From sima
/*
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
*/