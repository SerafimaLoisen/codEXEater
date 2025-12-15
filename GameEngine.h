#pragma once
#include "Player.h"
#include "Projectile.h" 
#include "Platform.h"
#include <vector>
#include <memory>
#include <map>
#include "Boss.h"
#include "BossRoot.h"
#include "IClonable.h"

class ComponentsBasedEntity;

class GameEngine {
private:
    std::unique_ptr<Player> player;
    std::vector<std::shared_ptr<Projectile>> projectiles;
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

    static GameEngine* instance;

#pragma region ENEMIES

private:

    typedef std::map<std::string, std::shared_ptr<ComponentsBasedEntity>> HostileEntitiesPrefabs;
    typedef std::shared_ptr<ComponentsBasedEntity> CBE;
    
    HostileEntitiesPrefabs GlobalHostileEntitiesPrefabs;
    HostileEntitiesPrefabs LevelBasedHostileEntitiesPrefabs;
    std::vector<std::shared_ptr<ComponentsBasedEntity>> hostileEntitiesToProcess;
    std::vector<char> reservedSymbols = { '=', ' ', '^', '>', '<', 'v', 'p', 's' };
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

    // Инициализация с выбором уровня
    void initialize(const std::string& levelName = "tutorial");

    // Доступ к активному экземпляру GameEngine
    static GameEngine* getInstance();

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