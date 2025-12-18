#include "GameEngine.h"
#include "ConfigManager.h"
#include "GraphicsManager.h"
#include "UIManager.h"
#include "Bullet.h"
#include "ParryBullet.h"
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <windows.h>

#include "Logger.h"
#include "SidePlatform.h"

#pragma region ENEMIES
#include "ComponentsBasedEntity.h"
#include "FollowLineTrajectoryComponent.h"
#include "DealDamageOnOverlapComponent.h"
#include "TakeDamageOnOverlapComponent.h"
#include "EmitProjectilesComponent.h"
#pragma endregion


GameEngine* GameEngine::instance = nullptr;

GameEngine* GameEngine::getInstance() {
    return instance;
}

GameEngine::GameEngine()
    : player(nullptr), bulletSpawnTimer(0), gameRunning(true), score(0),
    screenWidth(0), screenHeight(0), parryBulletSpeed(0), parryRange(5),
    currentLevel("tutorial"), bossMode(false), camera(nullptr),
    currentCheckpoint(nullptr), levelStartX(0), levelStartY(0) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    instance = this;
}

void GameEngine::initialize(const std::string& levelName) {
    setGameRunning(true);

    ConfigManager::initialize();
    UIManager::initialize();
    loadLevel(levelName);
}

void GameEngine::loadLevel(const std::string& levelName) {
    Logger::Log("=== GameEngine::loadLevel('" + levelName + "') ===");

    currentLevel = levelName;
    bossMode = (levelName == "boss");

    checkpoints.clear();
    currentCheckpoint = nullptr;

    auto& config = ConfigManager::getInstance();
    screenWidth = config.getScreenWidth();
    screenHeight = config.getScreenHeight();
    parryBulletSpeed = config.getParryBulletSpeed();
    parryRange = config.getParryRange();

    Logger::Log("Config loaded: screen=" + std::to_string(screenWidth) + "x" + std::to_string(screenHeight));

    loadGraphicsForLevel();
    normalizeUIFrame();

    //removeWinSymbolsFromUIFrame();

    int worldWidth = config.getWorldWidth(levelName);
    int worldHeight = config.getWorldHeight(levelName);

    Logger::Log("World size calculated: " + std::to_string(worldWidth) + "x" + std::to_string(worldHeight));

    camera = std::make_unique<Camera>(
        config.getViewportWidth(),
        config.getViewportHeight(),
        worldWidth,
        worldHeight
    );

    Logger::Log("Camera created: viewport=" +
        std::to_string(config.getViewportWidth()) + "x" +
        std::to_string(config.getViewportHeight()));

    projectiles.clear();
    platforms.clear();
    sidePlatforms.clear();
    hostileEntitiesToProcess.clear();
    checkpoints.clear();

    createPlatformsFromUIFrame();
    createCheckpointsFromUIFrame();

    int spawnX = -1;
    int spawnY = -1;
    bool foundSpawnPoint = findPlayerSpawn(spawnX, spawnY);

    if (!foundSpawnPoint) {
        spawnX = config.getPlayerStartX();
        spawnY = config.getPlayerStartY();
        Logger::Log("Player spawn 'P' not found, using config: (" +
            std::to_string(spawnX) + ", " + std::to_string(spawnY) + ")");
    }
    else {
        Logger::Log("Player spawn found at 'P': (" +
            std::to_string(spawnX) + ", " + std::to_string(spawnY) + ")");

        removeSpawnPointFromUIFrame(spawnX, spawnY);
    }

    levelStartX = spawnX;
    levelStartY = spawnY;

    const auto& uiFrame = GraphicsManager::getGraphic("UIFrame");
    if (!foundSpawnPoint && !uiFrame.empty()) {
        bool foundSafeSpot = false;
        for (int y = worldHeight - 5; y > 0 && !foundSafeSpot; y--) {
            if (y < uiFrame.size()) {
                for (int x = spawnX - 10; x < spawnX + 10 && x < uiFrame[y].length(); x++) {
                    if (x >= 0 && uiFrame[y][x] == '=') {
                        spawnX = x;
                        spawnY = y - 5;
                        foundSafeSpot = true;
                        break;
                    }
                }
            }
        }
    }

    player = std::make_shared<Player>(spawnX, spawnY, worldWidth, worldHeight);

    Logger::Log("Player created at world coords: (" +
        std::to_string(spawnX) + ", " + std::to_string(spawnY) + ")");

    camera->centerOn(player->getX(), player->getY());

    Logger::Log("Camera centered on player: camera(" +
        std::to_string(camera->getX()) + ", " + std::to_string(camera->getY()) + ")");

    std::cout << "=== Level '" << levelName << "' loaded ===" << std::endl;
    std::cout << "World size: " << worldWidth << "x" << worldHeight << std::endl;
    std::cout << "Viewport: " << camera->getViewportWidth() << "x" << camera->getViewportHeight() << std::endl;
    std::cout << "Player start: (" << spawnX << ", " << spawnY << ")" << std::endl;
	
#pragma region CREATE_ENEMIES
    DefineGlobalHostileEntitiesPrefabs();
    // Создаём врагов для уровня
    CreateHostileEntitiesFromLevelMap(DefineLevelBasedHostileEntitiesPrefabs(0));
#pragma endregion
}

void GameEngine::loadGraphicsForLevel() {
    GraphicsManager::loadGraphics("graphics/player.txt", "player");
    GraphicsManager::loadGraphics("graphics/player_dodge.txt", "player_dodge");
    GraphicsManager::loadGraphics("graphics/bullet.txt", "bullet");
    GraphicsManager::loadGraphics("graphics/parry_bullet.txt", "parry_bullet");

#pragma region LOAD_HOSTILE_SPRITES
    GraphicsManager::loadGraphics("graphics/hostile/enemy_default.txt", "enemy_default");
#pragma endregion

    // Графика для конкретного уровня
    if (currentLevel == "boss") {
        GraphicsManager::loadGraphics("graphics/boss/boss_frame.txt", "UIFrame");
    }
    else {
        GraphicsManager::loadGraphics("graphics/levels/" + currentLevel + ".txt", "UIFrame");
    }
}

#pragma region DEFINE_HOSTILE_ENTITIES

void GameEngine::DefineGlobalHostileEntitiesPrefabs() {
    
    // COMPONENTS CONFIGS

    DealDamageOnOverlapComponentConfig instantKillOnOverlap = DealDamageOnOverlapComponentConfig(1, 0);

    // PREFABS
    
    CBE spikeUp = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5);
    spikeUp->AddComponent(*new DealDamageOnOverlapComponent(spikeUp.get(), *getPlayerPtr(), instantKillOnOverlap));
    CBE spikeDown = std::shared_ptr<ComponentsBasedEntity>(spikeUp->clone(20, 5));
    CBE spikeRight = std::shared_ptr<ComponentsBasedEntity>(spikeUp->clone(20, 5));
    CBE spikeLeft = std::shared_ptr<ComponentsBasedEntity>(spikeUp->clone(20, 5));
    
    // BIND SYMBOL_ID TO PREFAB

    GlobalHostileEntitiesPrefabs["^"] = spikeUp;
    GlobalHostileEntitiesPrefabs["v"] = spikeDown;
    GlobalHostileEntitiesPrefabs["<"] = spikeLeft;
    GlobalHostileEntitiesPrefabs[">"] = spikeRight;
}

std::shared_ptr<GameEngine::HostileEntitiesPrefabs> GameEngine::DefineLevelBasedHostileEntitiesPrefabs(int levelIndex)
{
    // COMPONENTS CONFIGS
    
    // Movement on Axis X (to the left and to the right) with Speed of 1, Start Direction of 1 (to the right) and with the Rest Time of 12
    FollowLineTrajectoryComponentConfig horizontalMovement = FollowLineTrajectoryComponentConfig(false, 1, { 4, 4 }, -1, Axis::X, 12);
    FollowLineTrajectoryComponentConfig horizontalMovementShorter = FollowLineTrajectoryComponentConfig(false, 1, { 2, 2 }, -1, Axis::X, 12);

    FollowLineTrajectoryComponentConfig verticalMovement = FollowLineTrajectoryComponentConfig(false, 1, { 4, 0 }, -1, Axis::Y, 12);
    
    // { TO LEFT OFFSET, TO RIGHT OFFSET } FOR X AXIS
    // { TO TOP OFFSET, TO BOTTOM OFFSET } FOR Y AXIS
    FollowLineTrajectoryComponentConfig upSpikeMovement = FollowLineTrajectoryComponentConfig(false, 1, { 2, 0 }, 1, Axis::Y, 20, 0);
    FollowLineTrajectoryComponentConfig upSpikeMovement2 = FollowLineTrajectoryComponentConfig(false, 1, { 2, 0 }, 1, Axis::Y, 20, 10);
    FollowLineTrajectoryComponentConfig upSpikeMovement3 = FollowLineTrajectoryComponentConfig(false, 1, { 2, 0 }, 1, Axis::Y, 20, 20);
    FollowLineTrajectoryComponentConfig upSpikeMovement4 = FollowLineTrajectoryComponentConfig(false, 1, { 2, 0 }, 1, Axis::Y, 60, 0);

    FollowLineTrajectoryComponentConfig downSpikeMovement = FollowLineTrajectoryComponentConfig(false, 1, { 0, 2 }, 1, Axis::Y, 20, 0);
    FollowLineTrajectoryComponentConfig downSpikeMovement2 = FollowLineTrajectoryComponentConfig(false, 1, { 0, 2 }, 1, Axis::Y, 20, 10);
    FollowLineTrajectoryComponentConfig downSpikeMovement3 = FollowLineTrajectoryComponentConfig(false, 1, { 0, 2 }, 1, Axis::Y, 20, 20);
    FollowLineTrajectoryComponentConfig downSpikeMovement4 = FollowLineTrajectoryComponentConfig(false, 1, { 0, 2 }, 1, Axis::Y, 60, 0);

    FollowLineTrajectoryComponentConfig leftSpikeMovement = FollowLineTrajectoryComponentConfig(false, 1, { 7, 0 }, 1, Axis::X, 20, 0);
    FollowLineTrajectoryComponentConfig leftSpikeMovementWithStartDelay = FollowLineTrajectoryComponentConfig(false, 1, { 0, 97 }, 1, Axis::X, 20, 0);
    FollowLineTrajectoryComponentConfig rightSpikeMovement = FollowLineTrajectoryComponentConfig(false, 1, { 0, 7 }, 1, Axis::X, 20, 0);
    FollowLineTrajectoryComponentConfig rightSpikeMovementWithStartDelay = FollowLineTrajectoryComponentConfig(false, 1, { 104, 0 }, 1, Axis::X, 20, 0);

    FollowLineTrajectoryComponentConfig rightMovement = FollowLineTrajectoryComponentConfig(false, 1, { 99, 0 }, 1, Axis::X, 0, 0);
    
    // 
    DealDamageOnOverlapComponentConfig defaultOverlapDamage = DealDamageOnOverlapComponentConfig();
    DealDamageOnOverlapComponentConfig instantKillOnOverlap = DealDamageOnOverlapComponentConfig(1, 0);

    // Emit in the -1 direction on Axis X with Projectile Speed of 2 for MaxTravelDistance of 40 units, 3 Projectiles in a Row with Time Between Emissions=3 and Time Between Sequences of Emissions=6
    EmitProjectilesComponentConfig upEmit = EmitProjectilesComponentConfig({ 0, -1 }, 2, 10, 3, 1, 20, 0);
    EmitProjectilesComponentConfig downEmit = EmitProjectilesComponentConfig({ 0, 1 }, 2, 10, 3, 1, 20, 0);
    EmitProjectilesComponentConfig rightEmit = EmitProjectilesComponentConfig({ 1, 0 }, 2, 10, 3, 1, 40, 0); // ->
    EmitProjectilesComponentConfig leftEmit = EmitProjectilesComponentConfig({ -1, 0 }, 2, 10, 3, 1, 40, 0); // <-

    EmitProjectilesComponentConfig upEmit1 = EmitProjectilesComponentConfig({ 0, -1 }, 1, 10, 10, 1, 20, 0);
    EmitProjectilesComponentConfig downEmit1 = EmitProjectilesComponentConfig({ 0, 1 }, 1, 10, 10, 1, 20, 0);
    EmitProjectilesComponentConfig rightEmit1 = EmitProjectilesComponentConfig({ 1, 0 }, 1, 10, 5, 1, 20, 0); // ->
    EmitProjectilesComponentConfig leftEmit1 = EmitProjectilesComponentConfig({ -1, 0 }, 1, 10, 5, 1, 20, 0); // <-

    EmitProjectilesComponentConfig upEmit2 = EmitProjectilesComponentConfig({ 0, -1 }, 2, 10, 3, 1, 20, 20);
    EmitProjectilesComponentConfig downEmit2 = EmitProjectilesComponentConfig({ 0, 1 }, 2, 10, 3, 1, 20, 20);
    EmitProjectilesComponentConfig rightEmit2 = EmitProjectilesComponentConfig({ 1, 0 }, 2, 10, 3, 1, 40, 20); // ->
    EmitProjectilesComponentConfig leftEmit2 = EmitProjectilesComponentConfig({ -1, 0 }, 2, 10, 3, 1, 40, 20); // <-

    EmitProjectilesComponentConfig upEmit3 = EmitProjectilesComponentConfig({ 0, -1 }, 2, 10, 3, 1, 20, 40);
    EmitProjectilesComponentConfig downEmit3 = EmitProjectilesComponentConfig({ 0, 1 }, 2, 10, 3, 1, 20, 40);
    EmitProjectilesComponentConfig rightEmit3 = EmitProjectilesComponentConfig({ 1, 0 }, 2, 10, 3, 1, 40, 40); // ->
    EmitProjectilesComponentConfig leftEmit3 = EmitProjectilesComponentConfig({ -1, 0 }, 2, 10, 3, 1, 40, 40); // <-

    EmitProjectilesComponentConfig upEmit4 = EmitProjectilesComponentConfig({ 0, -1 }, 1, 4, 3, 1, 0, 0);
    EmitProjectilesComponentConfig downEmit4 = EmitProjectilesComponentConfig({ 0, 1 }, 1, 4, 3, 1, 0, 0);
    EmitProjectilesComponentConfig rightEmit4 = EmitProjectilesComponentConfig({ 1, 0 }, 1, 7, 3, 1, 0, 0); // ->
    EmitProjectilesComponentConfig leftEmit4 = EmitProjectilesComponentConfig({ -1, 0 }, 1, 7, 3, 1, 0, 0); // <-

    EmitProjectilesComponentConfig rightEmit5 = EmitProjectilesComponentConfig({ 1, 0 }, 1, 97, 3, 1, 0, 0); // ->

    EmitProjectilesComponentConfig upEmit7 = EmitProjectilesComponentConfig({ 0, -1 }, 1, 1, 5, 0, 0, 0);
    EmitProjectilesComponentConfig downEmit7 = EmitProjectilesComponentConfig({ 0, 1 }, 1, 1, 5, 0, 0, 0);
    EmitProjectilesComponentConfig rightEmit7 = EmitProjectilesComponentConfig({ 1, 0 }, 2, 10, 3, 1, 0, 0); // ->
    EmitProjectilesComponentConfig leftEmit7 = EmitProjectilesComponentConfig({ -1, 0 }, 2, 10, 3, 1, 0, 0); // <-
    
    EmitProjectilesComponentConfig upEmit10 = EmitProjectilesComponentConfig({ 0, -1 }, 2, 50, 1, 1, 50, 0);
    EmitProjectilesComponentConfig upEmit11 = EmitProjectilesComponentConfig({ 0, -1 }, 2, 50, 1, 1, 50, 5);
    EmitProjectilesComponentConfig upEmit12 = EmitProjectilesComponentConfig({ 0, -1 }, 2, 50, 1, 1, 50, 10);
    EmitProjectilesComponentConfig upEmit13 = EmitProjectilesComponentConfig({ 0, -1 }, 2, 50, 1, 1, 50, 15);
    EmitProjectilesComponentConfig upEmit14 = EmitProjectilesComponentConfig({ 0, -1 }, 2, 50, 1, 1, 50, 25);
    EmitProjectilesComponentConfig upEmit15 = EmitProjectilesComponentConfig({ 0, -1 }, 2, 50, 1, 1, 50, 30);
    EmitProjectilesComponentConfig upEmit16 = EmitProjectilesComponentConfig({ 0, -1 }, 2, 50, 1, 1, 50, 35);
    EmitProjectilesComponentConfig upEmit17 = EmitProjectilesComponentConfig({ 0, -1 }, 2, 50, 1, 1, 50, 40);
    EmitProjectilesComponentConfig upEmit18 = EmitProjectilesComponentConfig({ 0, -1 }, 2, 50, 1, 1, 50, 45);
    EmitProjectilesComponentConfig upEmit19 = EmitProjectilesComponentConfig({ 0, -1 }, 2, 50, 1, 1, 50, 50);

    EmitProjectilesComponentConfig upEmitEnemy = EmitProjectilesComponentConfig({ 0, -1 }, 2, 25, 3, 1, 20, 0);
    EmitProjectilesComponentConfig downEmitEnemy = EmitProjectilesComponentConfig({ 0, 1 }, 2, 25, 3, 1, 20, 0);
    EmitProjectilesComponentConfig rightEmitEnemy = EmitProjectilesComponentConfig({ 1, 0 }, 2, 25, 3, 1, 20, 0); // ->
    EmitProjectilesComponentConfig leftEmitEnemy = EmitProjectilesComponentConfig({ -1, 0 }, 2, 25, 3, 1, 20, 0); // <-

    EmitProjectilesComponentConfig emitTowardsTarget = EmitProjectilesComponentConfig({ -1, 0 }, 2, 40, 3, 3, 18, 0, true, false);

    std::shared_ptr<HostileEntitiesPrefabs> prefabs = std::make_shared<HostileEntitiesPrefabs>();

    switch (levelIndex) {
        case 0:
        {
            // LEVEL LOCAL PREFABS

            // ENEMY PREFAB
            std::shared_ptr<ComponentsBasedEntity> baseEnemy = std::make_shared<ComponentsBasedEntity>(20, 5, 5, 3, 2);
            baseEnemy->AddComponent(*new DealDamageOnOverlapComponent(baseEnemy.get(), *getPlayerPtr(), defaultOverlapDamage));
            baseEnemy->AddComponent(*new TakeDamageOnOverlapComponent(baseEnemy.get(), playerProjectiles));

            std::shared_ptr<ComponentsBasedEntity> moveEnemy = std::make_shared<ComponentsBasedEntity>(20, 5, 5, 3, 3);
            moveEnemy->AddComponent(*new FollowLineTrajectoryComponent(moveEnemy.get(), horizontalMovement));
            moveEnemy->AddComponent(*new DealDamageOnOverlapComponent(moveEnemy.get(), *getPlayerPtr(), defaultOverlapDamage));
            moveEnemy->AddComponent(*new TakeDamageOnOverlapComponent(moveEnemy.get(), playerProjectiles));

            std::shared_ptr<ComponentsBasedEntity> attackEnemy = std::make_shared<ComponentsBasedEntity>(20, 5, 5, 3, 3, true);
            attackEnemy->AddComponent(*new DealDamageOnOverlapComponent(attackEnemy.get(), *getPlayerPtr(), defaultOverlapDamage));
            attackEnemy->AddComponent(*new TakeDamageOnOverlapComponent(attackEnemy.get(), playerProjectiles, 1));
            attackEnemy->AddComponent(*new EmitProjectilesComponent(attackEnemy.get(), leftEmitEnemy, projectiles, getPlayerPtr()));

            std::shared_ptr<ComponentsBasedEntity> attackOnPlayerEnemy = std::make_shared<ComponentsBasedEntity>(20, 5, 5, 3, 3, true);
            attackOnPlayerEnemy->AddComponent(*new DealDamageOnOverlapComponent(attackOnPlayerEnemy.get(), *getPlayerPtr(), defaultOverlapDamage));
            attackOnPlayerEnemy->AddComponent(*new TakeDamageOnOverlapComponent(attackOnPlayerEnemy.get(), playerProjectiles, 1));
            attackOnPlayerEnemy->AddComponent(*new EmitProjectilesComponent(attackOnPlayerEnemy.get(), emitTowardsTarget, projectiles, getPlayerPtr()));

            std::shared_ptr<ComponentsBasedEntity> attackBackEnemy = std::make_shared<ComponentsBasedEntity>(20, 5, 5, 3, 3, true);
            attackBackEnemy->AddComponent(*new DealDamageOnOverlapComponent(attackBackEnemy.get(), *getPlayerPtr(), defaultOverlapDamage));
            attackBackEnemy->AddComponent(*new TakeDamageOnOverlapComponent(attackBackEnemy.get(), playerProjectiles, 1));
            attackBackEnemy->AddComponent(*new EmitProjectilesComponent(attackBackEnemy.get(), rightEmitEnemy, projectiles, getPlayerPtr()));

            std::shared_ptr<ComponentsBasedEntity> jumpEnemy = std::make_shared<ComponentsBasedEntity>(20, 5, 5, 3, 3);
            jumpEnemy->AddComponent(*new FollowLineTrajectoryComponent(jumpEnemy.get(), verticalMovement));
            jumpEnemy->AddComponent(*new DealDamageOnOverlapComponent(jumpEnemy.get(), *getPlayerPtr(), defaultOverlapDamage));
            jumpEnemy->AddComponent(*new TakeDamageOnOverlapComponent(jumpEnemy.get(), playerProjectiles));

            // PROJECTILES EMITTER PREFAB
            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> downProjectilesEmitter = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> rightProjectilesEmitter = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> leftProjectilesEmitter = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});

            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter1 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> downProjectilesEmitter1 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> rightProjectilesEmitter1 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> leftProjectilesEmitter1 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});

            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter2 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> downProjectilesEmitter2 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> rightProjectilesEmitter2 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> leftProjectilesEmitter2 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});

            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter3 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> downProjectilesEmitter3 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> rightProjectilesEmitter3 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> leftProjectilesEmitter3 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});

            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter4 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> downProjectilesEmitter4 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> rightProjectilesEmitter4 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> leftProjectilesEmitter4 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});

            std::shared_ptr<ComponentsBasedEntity> rightProjectilesEmitter5 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});

            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter7 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> downProjectilesEmitter7 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> rightProjectilesEmitter7 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> leftProjectilesEmitter7 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});

            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter10 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter11 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter12 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter13 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter14 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter15 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter16 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter17 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter18 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            std::shared_ptr<ComponentsBasedEntity> upProjectilesEmitter19 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});

            upProjectilesEmitter->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter.get(), upEmit, projectiles, getPlayerPtr()));
            downProjectilesEmitter->AddComponent(*new EmitProjectilesComponent(downProjectilesEmitter.get(), downEmit, projectiles, getPlayerPtr()));
            rightProjectilesEmitter->AddComponent(*new EmitProjectilesComponent(rightProjectilesEmitter.get(), rightEmit, projectiles, getPlayerPtr()));
            leftProjectilesEmitter->AddComponent(*new EmitProjectilesComponent(leftProjectilesEmitter.get(), leftEmit, projectiles, getPlayerPtr()));

            upProjectilesEmitter1->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter1.get(), upEmit1, projectiles, getPlayerPtr()));
            downProjectilesEmitter1->AddComponent(*new EmitProjectilesComponent(downProjectilesEmitter1.get(), downEmit1, projectiles, getPlayerPtr()));
            rightProjectilesEmitter1->AddComponent(*new EmitProjectilesComponent(rightProjectilesEmitter1.get(), rightEmit1, projectiles, getPlayerPtr()));
            leftProjectilesEmitter1->AddComponent(*new EmitProjectilesComponent(leftProjectilesEmitter1.get(), leftEmit1, projectiles, getPlayerPtr()));

            upProjectilesEmitter2->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter2.get(), upEmit2, projectiles, getPlayerPtr()));
            downProjectilesEmitter2->AddComponent(*new EmitProjectilesComponent(downProjectilesEmitter2.get(), downEmit2, projectiles, getPlayerPtr()));
            rightProjectilesEmitter2->AddComponent(*new EmitProjectilesComponent(rightProjectilesEmitter2.get(), rightEmit2, projectiles, getPlayerPtr()));
            leftProjectilesEmitter2->AddComponent(*new EmitProjectilesComponent(leftProjectilesEmitter2.get(), leftEmit2, projectiles, getPlayerPtr()));

            upProjectilesEmitter3->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter3.get(), upEmit3, projectiles, getPlayerPtr()));
            downProjectilesEmitter3->AddComponent(*new EmitProjectilesComponent(downProjectilesEmitter3.get(), downEmit3, projectiles, getPlayerPtr()));
            rightProjectilesEmitter3->AddComponent(*new EmitProjectilesComponent(rightProjectilesEmitter3.get(), rightEmit3, projectiles, getPlayerPtr()));
            leftProjectilesEmitter3->AddComponent(*new EmitProjectilesComponent(leftProjectilesEmitter3.get(), leftEmit3, projectiles, getPlayerPtr()));

            upProjectilesEmitter4->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter4.get(), upEmit4, projectiles, getPlayerPtr()));
            downProjectilesEmitter4->AddComponent(*new EmitProjectilesComponent(downProjectilesEmitter4.get(), downEmit4, projectiles, getPlayerPtr()));
            rightProjectilesEmitter4->AddComponent(*new EmitProjectilesComponent(rightProjectilesEmitter4.get(), rightEmit4, projectiles, getPlayerPtr()));
            leftProjectilesEmitter4->AddComponent(*new EmitProjectilesComponent(leftProjectilesEmitter4.get(), leftEmit4, projectiles, getPlayerPtr()));

            rightProjectilesEmitter5->AddComponent(*new EmitProjectilesComponent(rightProjectilesEmitter5.get(), rightEmit5, projectiles, getPlayerPtr()));

            upProjectilesEmitter7->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter7.get(), upEmit7, projectiles, getPlayerPtr()));
            downProjectilesEmitter7->AddComponent(*new EmitProjectilesComponent(downProjectilesEmitter7.get(), downEmit7, projectiles, getPlayerPtr()));
            rightProjectilesEmitter7->AddComponent(*new EmitProjectilesComponent(rightProjectilesEmitter7.get(), rightEmit7, projectiles, getPlayerPtr()));
            leftProjectilesEmitter7->AddComponent(*new EmitProjectilesComponent(leftProjectilesEmitter7.get(), leftEmit7, projectiles, getPlayerPtr()));

            upProjectilesEmitter7->AddComponent(*new FollowLineTrajectoryComponent(upProjectilesEmitter7.get(), rightMovement));
            downProjectilesEmitter7->AddComponent(*new FollowLineTrajectoryComponent(downProjectilesEmitter7.get(), rightMovement));
            rightProjectilesEmitter7->AddComponent(*new FollowLineTrajectoryComponent(rightProjectilesEmitter7.get(), rightMovement));
            leftProjectilesEmitter7->AddComponent(*new FollowLineTrajectoryComponent(leftProjectilesEmitter7.get(), rightMovement));

            upProjectilesEmitter10->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter10.get(), upEmit10, projectiles, getPlayerPtr()));
            upProjectilesEmitter11->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter11.get(), upEmit11, projectiles, getPlayerPtr()));
            upProjectilesEmitter12->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter12.get(), upEmit12, projectiles, getPlayerPtr()));
            upProjectilesEmitter13->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter13.get(), upEmit13, projectiles, getPlayerPtr()));
            upProjectilesEmitter14->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter14.get(), upEmit14, projectiles, getPlayerPtr()));
            upProjectilesEmitter15->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter15.get(), upEmit15, projectiles, getPlayerPtr()));
            upProjectilesEmitter16->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter16.get(), upEmit16, projectiles, getPlayerPtr()));
            upProjectilesEmitter17->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter17.get(), upEmit17, projectiles, getPlayerPtr()));
            upProjectilesEmitter18->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter18.get(), upEmit18, projectiles, getPlayerPtr()));
            upProjectilesEmitter19->AddComponent(*new EmitProjectilesComponent(upProjectilesEmitter19.get(), upEmit19, projectiles, getPlayerPtr()));

            //leftSpikeMovementWithStartDelay

            // CUSTOM SPIKES
            std::shared_ptr<ComponentsBasedEntity> upSpikeCustom = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false);
            std::shared_ptr<ComponentsBasedEntity> upSpikeCustom2 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false);
            std::shared_ptr<ComponentsBasedEntity> upSpikeCustom3 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false);
            std::shared_ptr<ComponentsBasedEntity> upSpikeCustom4 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false);

            std::shared_ptr<ComponentsBasedEntity> downSpikeCustom = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false);
            std::shared_ptr<ComponentsBasedEntity> downSpikeCustom2 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false);
            std::shared_ptr<ComponentsBasedEntity> downSpikeCustom3 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false);
            std::shared_ptr<ComponentsBasedEntity> downSpikeCustom4 = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false);
            
            std::shared_ptr<ComponentsBasedEntity> rightSpikeCustom = std::shared_ptr<ComponentsBasedEntity>(upSpikeCustom->clone(0,0));
            std::shared_ptr<ComponentsBasedEntity> rightSpikeCustomWithStartDelay = std::shared_ptr<ComponentsBasedEntity>(upSpikeCustom->clone(0,0));
            std::shared_ptr<ComponentsBasedEntity> leftSpikeCustom = std::shared_ptr<ComponentsBasedEntity>(upSpikeCustom->clone(0,0));
            std::shared_ptr<ComponentsBasedEntity> leftSpikeCustomWithStartDelay = std::shared_ptr<ComponentsBasedEntity>(upSpikeCustom->clone(0,0));
            
            upSpikeCustom->AddComponent(*new DealDamageOnOverlapComponent(upSpikeCustom.get(), *getPlayerPtr(), defaultOverlapDamage));
            upSpikeCustom2->AddComponent(*new DealDamageOnOverlapComponent(upSpikeCustom2.get(), *getPlayerPtr(), defaultOverlapDamage));
            upSpikeCustom3->AddComponent(*new DealDamageOnOverlapComponent(upSpikeCustom3.get(), *getPlayerPtr(), defaultOverlapDamage));
            upSpikeCustom4->AddComponent(*new DealDamageOnOverlapComponent(upSpikeCustom4.get(), *getPlayerPtr(), defaultOverlapDamage));

            downSpikeCustom->AddComponent(*new DealDamageOnOverlapComponent(downSpikeCustom.get(), *getPlayerPtr(), defaultOverlapDamage));
            downSpikeCustom2->AddComponent(*new DealDamageOnOverlapComponent(downSpikeCustom2.get(), *getPlayerPtr(), defaultOverlapDamage));
            downSpikeCustom3->AddComponent(*new DealDamageOnOverlapComponent(downSpikeCustom3.get(), *getPlayerPtr(), defaultOverlapDamage));
            downSpikeCustom4->AddComponent(*new DealDamageOnOverlapComponent(downSpikeCustom4.get(), *getPlayerPtr(), defaultOverlapDamage));

            rightSpikeCustom->AddComponent(*new DealDamageOnOverlapComponent(rightSpikeCustom.get(), *getPlayerPtr(), defaultOverlapDamage));
            rightSpikeCustomWithStartDelay->AddComponent(*new DealDamageOnOverlapComponent(rightSpikeCustomWithStartDelay.get(), *getPlayerPtr(), defaultOverlapDamage));
            leftSpikeCustom->AddComponent(*new DealDamageOnOverlapComponent(leftSpikeCustom.get(), *getPlayerPtr(), defaultOverlapDamage));
            leftSpikeCustomWithStartDelay->AddComponent(*new DealDamageOnOverlapComponent(leftSpikeCustomWithStartDelay.get(), *getPlayerPtr(), defaultOverlapDamage));

            upSpikeCustom->AddComponent(*new FollowLineTrajectoryComponent(upSpikeCustom.get(), upSpikeMovement));
            upSpikeCustom2->AddComponent(*new FollowLineTrajectoryComponent(upSpikeCustom2.get(), upSpikeMovement2));
            upSpikeCustom3->AddComponent(*new FollowLineTrajectoryComponent(upSpikeCustom3.get(), upSpikeMovement3));
            upSpikeCustom4->AddComponent(*new FollowLineTrajectoryComponent(upSpikeCustom4.get(), upSpikeMovement4));
            downSpikeCustom->AddComponent(*new FollowLineTrajectoryComponent(downSpikeCustom.get(), downSpikeMovement));
            downSpikeCustom2->AddComponent(*new FollowLineTrajectoryComponent(downSpikeCustom2.get(), downSpikeMovement2));
            downSpikeCustom3->AddComponent(*new FollowLineTrajectoryComponent(downSpikeCustom3.get(), downSpikeMovement3));
            downSpikeCustom4->AddComponent(*new FollowLineTrajectoryComponent(downSpikeCustom4.get(), downSpikeMovement4));
            rightSpikeCustom->AddComponent(*new FollowLineTrajectoryComponent(rightSpikeCustom.get(), rightSpikeMovement));
            rightSpikeCustomWithStartDelay->AddComponent(*new FollowLineTrajectoryComponent(rightSpikeCustomWithStartDelay.get(), rightSpikeMovementWithStartDelay));
            leftSpikeCustom->AddComponent(*new FollowLineTrajectoryComponent(leftSpikeCustom.get(), leftSpikeMovement));
            leftSpikeCustomWithStartDelay->AddComponent(*new FollowLineTrajectoryComponent(leftSpikeCustomWithStartDelay.get(), leftSpikeMovementWithStartDelay));


            // BIND SYMBOL_ID TO PREFAB

            (*prefabs)["e"] = baseEnemy;
            (*prefabs)["e2"] = moveEnemy;
            (*prefabs)["e3"] = attackEnemy;
            (*prefabs)["e4"] = attackOnPlayerEnemy;
            (*prefabs)["e5"] = attackBackEnemy;
            (*prefabs)["e6"] = jumpEnemy;
            
            (*prefabs)["w"] = upProjectilesEmitter;
            (*prefabs)["s"] = downProjectilesEmitter;
            (*prefabs)["a"] = leftProjectilesEmitter;
            (*prefabs)["d"] = rightProjectilesEmitter;

            (*prefabs)["w1"] = upProjectilesEmitter1;
            (*prefabs)["s1"] = downProjectilesEmitter1;
            (*prefabs)["a1"] = leftProjectilesEmitter1;
            (*prefabs)["d1"] = rightProjectilesEmitter1;

            (*prefabs)["w2"] = upProjectilesEmitter2;
            (*prefabs)["s2"] = downProjectilesEmitter2;
            (*prefabs)["a2"] = leftProjectilesEmitter2;
            (*prefabs)["d2"] = rightProjectilesEmitter2;

            (*prefabs)["w3"] = upProjectilesEmitter3;
            (*prefabs)["s3"] = downProjectilesEmitter3;
            (*prefabs)["a3"] = leftProjectilesEmitter3;
            (*prefabs)["d3"] = rightProjectilesEmitter3;

            (*prefabs)["w4"] = upProjectilesEmitter4;
            (*prefabs)["s4"] = downProjectilesEmitter4;
            (*prefabs)["a4"] = leftProjectilesEmitter4;
            (*prefabs)["d4"] = rightProjectilesEmitter4;

            (*prefabs)["d5"] = rightProjectilesEmitter5;

            (*prefabs)["w7"] = upProjectilesEmitter7;
            (*prefabs)["s7"] = downProjectilesEmitter7;
            (*prefabs)["a7"] = leftProjectilesEmitter7;
            (*prefabs)["d7"] = rightProjectilesEmitter7;

            (*prefabs)["wr"] = upProjectilesEmitter10;
            (*prefabs)["wt"] = upProjectilesEmitter11;
            (*prefabs)["wy"] = upProjectilesEmitter12;
            (*prefabs)["wu"] = upProjectilesEmitter13;
            (*prefabs)["wi"] = upProjectilesEmitter14;
            (*prefabs)["wo"] = upProjectilesEmitter15;
            (*prefabs)["wl"] = upProjectilesEmitter16;
            (*prefabs)["wk"] = upProjectilesEmitter17;
            (*prefabs)["wj"] = upProjectilesEmitter18;
            (*prefabs)["wh"] = upProjectilesEmitter19;

            (*prefabs)["^1"] = upSpikeCustom;
            (*prefabs)["^2"] = upSpikeCustom2;
            (*prefabs)["^3"] = upSpikeCustom3;
            (*prefabs)["^4"] = upSpikeCustom4;

            (*prefabs)["v1"] = downSpikeCustom;
            (*prefabs)["v2"] = downSpikeCustom2;
            (*prefabs)["v3"] = downSpikeCustom3;
            (*prefabs)["v4"] = downSpikeCustom4;

            (*prefabs)["<1"] = leftSpikeCustom;
            (*prefabs)["<2"] = leftSpikeCustomWithStartDelay;
            
            (*prefabs)[">1"] = rightSpikeCustom;
            (*prefabs)[">2"] = rightSpikeCustomWithStartDelay;
        }
        break;
        default:
            prefabs = DefineLevelBasedHostileEntitiesPrefabs(0);
        break;
    }

    return prefabs;
}

#pragma endregion

#pragma region CREATE_HOSTILE_ENTITIES
void GameEngine::CreateHostileEntitiesFromLevelMap(std::shared_ptr<HostileEntitiesPrefabs> levelBasedHostileEntitiesPrefabs, std::string levelMapName) {
    
    HostileEntitiesPrefabs curLevelHostileEntitiesPrefabs = *levelBasedHostileEntitiesPrefabs;

    auto& uiFrame = const_cast<std::vector<std::string>&>(GraphicsManager::getGraphic(levelMapName));

    // CREATE SPIKES
    for (int y = 0; y < uiFrame.size(); y++) {
        std::string& line = uiFrame[y];

        int scale = -1;
        int startCoord = -1;
        std::vector<int> initCoord = {0, 0};
        int direction = -1;
        char altSpriteSymbol = '?';
        std::string ID;

        for (int x = 0; x < line.length(); x++) {
            
            char c = line[x];

            // DEFINE DIRECTION
            if (direction < 0) {
                for (int i = 0; i < spikes.size(); i++) {
                    if (c == spikes[i]) {
                        
                        altSpriteSymbol = spikes[i];
                        ID = GetEntityIDFromUIFrame(x, y, uiFrame);
                        initCoord[0] = x;
                        initCoord[1] = y;

                        if (i <= 1) {
                            // HORIZONTAL SPIKES
                            startCoord = x;
                            direction = 0;
                        }
                        else {
                            // VERTICAL SPIKES
                            startCoord = y;
                            direction = 1;
                            for (int j = startCoord; j < uiFrame.size(); j++) {
                                c = uiFrame[j][x];
                                if (c != altSpriteSymbol) {
                                    scale = max(j - startCoord, 1);
                                    break;
                                }
                                uiFrame[j][x] = ' ';
                            }
                        }

                        break;
                    }
                }
            }

            if (direction == 0) {
                if (c != altSpriteSymbol) {
                    scale = max(x - startCoord, 1);
                }
                else line[x] = ' ';
            }

            if (scale > 0) {
                std::vector<std::string> altSprite;
                if (direction == 0)
                    altSprite.push_back(std::string(scale, altSpriteSymbol));
                else {
                    for (int i = 0; i < scale; i++)
                        altSprite.push_back(std::string(1, altSpriteSymbol));
                }
                
                std::shared_ptr<ComponentsBasedEntity> spikes;
                std::shared_ptr<ComponentsBasedEntity> prefab = GetPrefabFromID(ID, curLevelHostileEntitiesPrefabs);
                spikes = std::shared_ptr<ComponentsBasedEntity>(prefab->clone(initCoord[0], initCoord[1]));
                
                spikes->SetAltSprite(altSprite);
                spikes->SetUseAltSprite(true);
                if (direction == 0) spikes->SetWidth(scale);
                else spikes->SetHeight(scale);

                spikes->SetWidth(spikes->getWidth()-2); // Why do we have to subtract 2? (visual position of player is two spaces to the right of their's actual X coord)

                hostileEntitiesToProcess.push_back(spikes);

                scale = -1;
                startCoord = -1;
                initCoord = { 0, 0 };
                direction = -1;
                altSpriteSymbol = '?';
            }
          
        }
    }

    // CREATE HOSTILE ENTITIES
    for (int y = 0; y < uiFrame.size(); y++) {
        std::string& line = uiFrame[y];

        for (int x = 0; x < line.length(); x++) {

            std::string ID = std::string(1, line[x]);

            if (curLevelHostileEntitiesPrefabs.find(ID) != curLevelHostileEntitiesPrefabs.end()) {
                
                ID = GetEntityIDFromUIFrame(x, y, uiFrame);

                if (curLevelHostileEntitiesPrefabs.find(ID) == curLevelHostileEntitiesPrefabs.end())
                    continue;

                std::shared_ptr<ComponentsBasedEntity> hostileEntity;
                std::shared_ptr<ComponentsBasedEntity> prefab = GetPrefabFromID(ID, curLevelHostileEntitiesPrefabs);
                hostileEntity = std::shared_ptr<ComponentsBasedEntity>(prefab->clone(x - (prefab->getWidth()-1), y - (prefab->getHeight() - 1)));
                hostileEntitiesToProcess.push_back(hostileEntity);

                line[x] = ' ';
            }
        }
    }
}

std::shared_ptr<ComponentsBasedEntity> GameEngine::GetPrefabFromID(std::string ID, HostileEntitiesPrefabs& curLevelHostileEntitiesPrefabs) {
    
    std::shared_ptr<ComponentsBasedEntity> prefab = nullptr;

    if (curLevelHostileEntitiesPrefabs.find(ID) != curLevelHostileEntitiesPrefabs.end()) 
    {
        prefab = curLevelHostileEntitiesPrefabs[ID];
    }
    else if (GlobalHostileEntitiesPrefabs.find(ID) != GlobalHostileEntitiesPrefabs.end()) 
    {
        prefab = GlobalHostileEntitiesPrefabs[ID];
    }

    return prefab;
}

std::string GameEngine::GetEntityIDFromUIFrame(int x, int y, std::vector<std::string>& uiFrame)
{
    std::string& line = uiFrame[y];
    std::string ID = std::string(1, line[x]);

    std::vector<char*> neighbourCells;

    if (y > 0)
    {
        std::string& topLine = uiFrame[y - 1];

        if (x < topLine.length())
            neighbourCells.push_back(&uiFrame[y - 1][x]);
    }
    if (x < line.length() - 1) neighbourCells.push_back(&line[x + 1]);
    if (y < uiFrame.size() - 1)
    {
        std::string& bottomLine = uiFrame[y + 1];
        if (x < bottomLine.length())
            neighbourCells.push_back(&uiFrame[y + 1][x]);
    }
    if (x > 0) neighbourCells.push_back(&line[x - 1]);

    for (char* c : neighbourCells) {

        bool isIndex = true;

        for (char reserved : reservedSymbols) {
            bool inRange = ((*c >= 48) && (*c <= 57)) || ((*c >= 65) && (*c <= 90)) || ((*c >= 97) && (*c <= 122));
            if (*c == reserved || !inRange) {
                isIndex = false;
                break;
            }
        }

        if (isIndex)
        {
            ID += *c;
            *c = ' ';
            break;
        }
    }

    ID.erase(std::remove(ID.begin(), ID.end(), '\t'), ID.end());

    return ID;
}
#pragma endregion

void GameEngine::createPlatformsFromUIFrame() {
    const auto& uiFrame = GraphicsManager::getGraphic("UIFrame");

    for (int y = 0; y < uiFrame.size(); y++) {
        const std::string& line = uiFrame[y];

        int platformStart = -1;

        for (int x = 0; x < line.length(); x++) {
            char currentChar = line[x];

            if (currentChar == 'P' || currentChar == 'p') {
                if (platformStart != -1) {
                    int platformWidth = x - platformStart;
                    if (platformWidth > 0) {
                        platforms.push_back(std::make_shared<Platform>(platformStart, y, platformWidth));
                    }
                    platformStart = -1;
                }
                continue;
            }

            if (currentChar == '=') {
                if (platformStart == -1) {
                    platformStart = x;
                }
            }
            else if (currentChar == '#') {
                int platformHeight = 1;
                for (int checkY = y + 1; checkY < uiFrame.size(); checkY++) {
                    if (checkY < uiFrame.size() &&
                        x < uiFrame[checkY].length() &&
                        uiFrame[checkY][x] == '#') {
                        platformHeight++;
                    }
                    else {
                        break;
                    }
                }

                if (platformHeight > 0) {
                    sidePlatforms.push_back(std::make_shared<SidePlatform>(x, y, platformHeight));

                    platformStart = -1;
                    continue;
                }
            }
            else {
                if (platformStart != -1 && currentChar != '=') {
                    int platformWidth = x - platformStart;
                    if (platformWidth > 0) {
                        platforms.push_back(std::make_shared<Platform>(platformStart, y, platformWidth));
                    }
                    platformStart = -1;
                }
            }
        }

        if (platformStart != -1) {
            int platformWidth = line.length() - platformStart;
            if (platformWidth > 0) {
                platforms.push_back(std::make_shared<Platform>(platformStart, y, platformWidth));
            }
        }
    }
}

void GameEngine::removePlatformsFromUIFrame() {
    auto& uiFrame = const_cast<std::vector<std::string>&>(GraphicsManager::getGraphic("UIFrame"));

    for (int y = 0; y < uiFrame.size(); y++) {
        std::string& line = uiFrame[y];
        for (int x = 0; x < line.length(); x++) {
            if (line[x] == '=') {
                line[x] = ' ';
            }
        }
    }
}

//void GameEngine::spawnBoss() {
//    if (!bossMode) return;
//
//    auto& config = ConfigManager::getInstance();
//    boss = std::make_unique<Boss>(
//        config.getScreenWidth() - config.getBossWidth() - 5,
//        config.getScreenHeight() / 2 - config.getBossHeight() / 2,
//        config.getBossWidth(),
//        config.getBossHeight(),
//        config.getBossHealth(),
//        config.getBossPhase2HP(),
//        config.getBossPhase3HP(),
//        config.getBossBulletSpeed(),
//        config.getBossBulletColor(),
//        config.getBossBulletCooldown(),
//        config.getBossRootWarningDuration(),
//        config.getBossRootGrowDuration(),
//        config.getBossRootDamage(),
//        config.getBossRootCooldown(),
//        config.getBossRootColor()
//    );
//}


void GameEngine::update() {
    if (!player) return;

    checkWinCondition();

    handlePlayerCollisions();

    int activationRange = ConfigManager::getInstance().getCheckpointActivationRange();

    for (auto& checkpoint : checkpoints) {
        if (!checkpoint->isActive()) {
            int playerCenterX = player->getX() + player->getWidth() / 2;
            int playerCenterY = player->getY() + player->getHeight() / 2;

            int checkpointX = checkpoint->getX();
            int checkpointY = checkpoint->getY();

            // Используем радиус из конфига
            int diffX = abs(playerCenterX - checkpointX);
            int diffY = abs(playerCenterY - checkpointY);

            if (diffX <= activationRange && diffY <= activationRange) {
                Logger::Log("CHECKPOINT ACTIVATION CONDITION MET!");
                activateCheckpoint(checkpoint);
                break;
            }
        }
    }

    player->update();

    handlePlayerAttack();

    updateCamera();

    handlePlayerAttack();

    for (int i = projectiles.size() - 1; i >= 0; --i) {
        if (!projectiles[i]->isActive()) {
            projectiles.erase(projectiles.begin() + i);
        }
    }

    // Обновляем активные пули
    for (auto& bullet : projectiles) {
        if (bullet->isActive()) {
            bullet->update();

            auto& config = ConfigManager::getInstance();
            int worldWidth = config.getWorldWidth(currentLevel);
            int worldHeight = config.getWorldHeight(currentLevel);

            if (bullet->getX() < 0 ||
                bullet->getX() > worldWidth + 10 ||
                bullet->getY() < 0 ||
                bullet->getY() > worldHeight) {
                bullet->setActive(false);
            }
        }
    }

    // Спавн новых пуль
    /*bulletSpawnTimer++;
    int randomSpawnRate;*/

    //if (bossMode) {
    //    // Босс режим - чаще и сложнее
    //    /*updateBoss();
    //    checkBossCollisions();
    //    randomSpawnRate = 10 + (std::rand() % 15);*/
    //}
    //else {
    //    // Туториал - реже и проще
    //    randomSpawnRate = 20 + (std::rand() % 25);
    //}

    //if (bulletSpawnTimer >= randomSpawnRate) {
    //    spawnBullet();
    //    bulletSpawnTimer = 0;
    //}

    // Проверяем столкновения пуль с игроком
    checkCollisions();

    // Обрабатываем парирование
    handleParry();

    // Проверяем условие смерти (HP = 0)
    if (!player->isAlive()) {
        if (currentCheckpoint) {
            respawnAtCheckpoint();
        }
        else {
            respawnAtLevelStart();
        }
        player->heal(player->getMaxHealth());
    }

#pragma region UPDATE_ENEMIES
    for (auto& entity : hostileEntitiesToProcess) 
    {
        entity->update();

        if (entity->GetIsGravityEnabled())
        {
            for (const auto& platform : platforms)
            {
                if (entity->isCollidingWithPlatform(*platform))
                {
                    entity->setY(platform->getY() - entity->getHeight());
                    entity->SetVerticalVelocity(0);
                    break;
                }
            }

            // STOLEN FROM VIKA

            int currentX = entity->getX();
            int currentY = entity->getY();
            int playerWidth = entity->getWidth();
            float velocityX = entity->GetHorizontalVelocity();

            int predictedX = currentX + static_cast<int>(velocityX);

            bool collisionDetected = false;

            for (const auto& sidePlatform : sidePlatforms) {
                bool fromLeft = false;

                entity->setX(predictedX);

                if (entity->isCollidingWithSidePlatform(*sidePlatform, fromLeft)) {
                    collisionDetected = true;

                    entity->setX(currentX);

                    if (fromLeft) {
                        entity->setX(sidePlatform->getX() - playerWidth);
                    }
                    else {
                        entity->setX(sidePlatform->getX() + 1);
                    }

                    entity->setDirection(-1*entity->getDirection());

                    break;
                }

                entity->setX(currentX);
            }
        }

    }  
#pragma endregion
}

//void GameEngine::updateBoss() {
//    boss->update(*this);
//
//    // РћР±РЅРѕРІР»СЏРµРј РєРѕСЂРЅРё
//    for (int i = bossRoots.size() - 1; i >= 0; --i) {
//        bossRoots[i]->update();
//        if (bossRoots[i]->getHealth() <= 0) {  // РР»Рё РґСЂСѓРіРѕР№ РєСЂРёС‚РµСЂРёР№ СѓРґР°Р»РµРЅРёСЏ
//            bossRoots.erase(bossRoots.begin() + i);
//        }
//    }
//}

void GameEngine::spawnBullet() {
    bool isParryBullet = (std::rand() % 2 == 0);

    auto& config = ConfigManager::getInstance();
    int worldWidth = config.getWorldWidth(currentLevel);
    int worldHeight = config.getWorldHeight(currentLevel);

    int spawnX = worldWidth;
    int spawnY;

    if (bossMode) {
        spawnY = 5 + (std::rand() % (worldHeight - 10));
    }
    else {
        spawnY = 8 + (std::rand() % 5);
    }

    int direction = -1;

    if (isParryBullet) {
        projectiles.push_back(std::make_unique<ParryBullet>(spawnX, spawnY, direction));
    }
    else {
        projectiles.push_back(std::make_unique<Bullet>(spawnX, spawnY, direction));
    }
}

void GameEngine::handlePlayerCollisions() {
    if (!player) return;

    handlePlayerWorldCollisions();
    handlePlayerPlatformCollisions();
    handlePlayerSidePlatformCollisions();
}

void GameEngine::handlePlayerSidePlatformCollisions() {
    if (!player) return;

    if (fabs(player->getVelocityX()) < 0.1f) return;

    int currentX = player->getX();
    int currentY = player->getY();
    int playerWidth = player->getWidth();
    float velocityX = player->getVelocityX();

    int predictedX = currentX + static_cast<int>(velocityX);

    bool collisionDetected = false;

    for (const auto& sidePlatform : sidePlatforms) {
        bool fromLeft = false;

        player->setX(predictedX);

        if (player->isCollidingWithSidePlatform(*sidePlatform, fromLeft)) {
            collisionDetected = true;

            player->setX(currentX);

            if (fromLeft) {
                player->setX(sidePlatform->getX() - playerWidth);
            }
            else {
                player->setX(sidePlatform->getX() + sidePlatform->getWidth());
            }

            player->setVelocityX(0.0f);

            Logger::Log("Player collided with sidePlatform at (" +
                std::to_string(sidePlatform->getX()) + "," +
                std::to_string(sidePlatform->getY()) + ") from " +
                (fromLeft ? "left" : "right") +
                " | Player was at " + std::to_string(currentX) +
                ", predicted " + std::to_string(predictedX) +
                ", moved to " + std::to_string(player->getX()));

            break;
        }

        player->setX(currentX);
    }
}

void GameEngine::handlePlayerWorldCollisions() {
    if (!player) return;

    auto& config = ConfigManager::getInstance();
    int worldHeight = config.getWorldHeight(currentLevel);

    if (player->checkGroundCollision()) {
        player->setY(worldHeight - player->getHeight());
        player->setVelocityY(0);
        player->setOnGround(true);
    }
    else {
        player->setOnGround(false);
    }
}

void GameEngine::handlePlayerPlatformCollisions() {
    if (!player) return;

    bool foundPlatform = false;

    for (const auto& platform : platforms) {
        bool fromTop = false;

        if (player->isCollidingWithPlatform(*platform, fromTop)) {
            if (fromTop) {
                player->setY(platform->getY() - player->getHeight());
                player->setOnGround(true);
                foundPlatform = true;
            }
            else {
                player->setY(platform->getY() + platform->getHeight());
            }
            player->setVelocityY(0);
            break;
        }
    }

    if (!foundPlatform && !player->checkGroundCollision()) {
        player->setOnGround(false);
    }
}

void GameEngine::handlePlayerAttack() {
    auto newBullet = player->tryFire();
    if (newBullet) {
		
        // Добавляем в общий массив projectiles
        projectiles.push_back(newBullet);
        playerProjectiles.push_back(newBullet);
		
    }
}

void GameEngine::checkCollisions() {
    for (auto& bullet : projectiles) {
        if (!bullet->isActive()) continue;

        bool isPlayerBullet = (bullet->getColor() == ConfigManager::getInstance().getPlayerBulletColor());
        if (isPlayerBullet) continue;

        if (player->checkCollision(*bullet)) {
            if (player->getIsDodging()) {
                continue;
            }

            ParryBullet* parryBullet = dynamic_cast<ParryBullet*>(bullet.get());
            if (parryBullet && player->getIsParrying()) {
                bullet->setDirection(-bullet->getDirection());
                bullet->setSpeed(parryBulletSpeed * 2);
                score += 10;
                continue;
            }

            int healthBeforeDamage = player->getHealth();

            player->takeDamage(1);
            bullet->setActive(false);

            if (currentCheckpoint) {
                respawnAtCheckpoint();
            }
        }
    }
}

//void GameEngine::checkBossCollisions() {
//    if (!boss || !player) return;
//
//    // РџСЂРѕРІРµСЂСЏРµРј РїРѕРїР°РґР°РЅРёРµ РїСѓР»СЊ РёРіСЂРѕРєР° РІ Р±РѕСЃСЃР°
//    for (int i = projectiles.size() - 1; i >= 0; --i) {
//        if (!projectiles[i]->isActive()) continue;
//
//        bool isPlayerBullet = (projectiles[i]->getColor() ==
//            ConfigManager::getInstance().getPlayerBulletColor());
//
//        if (isPlayerBullet && boss->checkCollision(*projectiles[i])) {
//            boss->takeDamage(1);  // РљР°Р¶РґР°СЏ РїСѓР»СЏ РЅР°РЅРѕСЃРёС‚ 1 СѓСЂРѕРЅ
//            projectiles[i]->setActive(false);
//        }
//    }
//
//    // РџСЂРѕРІРµСЂСЏРµРј СЃС‚РѕР»РєРЅРѕРІРµРЅРёРµ РёРіСЂРѕРєР° СЃ РєРѕСЂРЅСЏРјРё Р±РѕСЃСЃР°
//    for (auto& root : bossRoots) {
//        if (player->checkCollision(*root)) {
//            player->takeDamage(root->getDamage());
//            // РњРѕР¶РЅРѕ СѓРґР°Р»РёС‚СЊ РєРѕСЂРµРЅСЊ РїРѕСЃР»Рµ РЅР°РЅРµСЃРµРЅРёСЏ СѓСЂРѕРЅР°
//            root->takeDamage(root->getHealth());  // РЈРЅРёС‡С‚РѕР¶Р°РµРј РєРѕСЂРµРЅСЊ
//        }
//    }
//}

void GameEngine::handleParry() {
    if (player->getIsParrying()) {
        for (auto& bullet : projectiles) {
            if (!bullet->isActive()) continue;

            if (isInParryRange(*bullet)) {
                ParryBullet* parryBullet = dynamic_cast<ParryBullet*>(bullet.get());
                if (parryBullet) {
                    bullet->setDirection(-bullet->getDirection());
                    bullet->setSpeed(parryBulletSpeed * 2);
                    score += 10;
                }
            }
        }
    }
}

bool GameEngine::isInParryRange(const GameObject& bullet) const {
    int playerRight = player->getX() + player->getWidth();
    return (bullet.getX() >= playerRight &&
        bullet.getX() <= playerRight + parryRange &&
        abs(bullet.getY() - player->getY()) <= 2);
}

void GameEngine::render() {
    if (!player || !camera) return;

    renderUIFrameWithCamera();

    for (auto& checkpoint : checkpoints) {
        if (camera->isInViewport(checkpoint->getX(), checkpoint->getY(), 1, 1)) {
            int screenX = camera->worldToScreenX(checkpoint->getX());
            int screenY = camera->worldToScreenY(checkpoint->getY());

            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            COORD coord;
            coord.X = screenX;
            coord.Y = screenY;
            SetConsoleCursorPosition(hConsole, coord);

            // Используем цвета из конфига
            int color;
            if (checkpoint->isActive()) {
                color = ConfigManager::getInstance().getCheckpointActiveColor();
                Logger::Log("Rendering ACTIVE checkpoint at (" +
                    std::to_string(screenX) + "," + std::to_string(screenY) +
                    ") with color " + std::to_string(color));
            }
            else {
                color = ConfigManager::getInstance().getCheckpointInactiveColor();
                Logger::Log("Rendering INACTIVE checkpoint at (" +
                    std::to_string(screenX) + "," + std::to_string(screenY) +
                    ") with color " + std::to_string(color));
            }

            SetConsoleTextAttribute(hConsole, color);
            std::cout << ConfigManager::getInstance().getCheckpointSymbol();
            SetConsoleTextAttribute(hConsole, 7);
        }
    }

    // Отрисовываем игрока
    if (camera->isInViewport(player->getX(), player->getY(),
        player->getWidth(), player->getHeight())) {
        int screenX = camera->worldToScreenX(player->getX());
        int screenY = camera->worldToScreenY(player->getY());
        player->renderAt(screenX, screenY);
    }

    // Отрисовываем пули
    for (auto& bullet : projectiles) {
        if (bullet->isActive() &&
            camera->isInViewport(bullet->getX(), bullet->getY(), 1, 1)) {
            int screenX = camera->worldToScreenX(bullet->getX());
            int screenY = camera->worldToScreenY(bullet->getY());
            bullet->renderAt(screenX, screenY);
        }
    }

    // Отрисовываем платформы
    //for (auto& platform : platforms) {
    //    if (platform->isActive() &&
    //        camera->isInViewport(platform->getX(), platform->getY(), 1, 1)) {
    //        int screenX = camera->worldToScreenX(platform->getX());
    //        int screenY = camera->worldToScreenY(platform->getY());
    //        platform->renderAt(screenX, screenY);
    //    }
    //}
    
    #pragma region RENDER_HOSTILE_ENTITIES
    for (auto& entity : hostileEntitiesToProcess) {
        if (entity->isActive() &&
            camera->isInViewport(entity->getX(), entity->getY(), entity->getWidth(), entity->getHeight())) {
            entity->renderFitViewport(camera->getX(), camera->getY(), camera->getViewportWidth(), camera->getViewportHeight());
        }
    }
    #pragma endregion
    
    //renderBoss();
    //
    // Отрисовываем подсказки и управление
    //UIManager::renderHints();
    //UIManager::renderControls();


    // Отрисовываем UI поверх всего
    UIManager::renderGameUI(*player, score);
}


void GameEngine::renderUIFrameWithCamera() {
    if (!camera) return;

    const auto& uiFrame = GraphicsManager::getGraphic("UIFrame");

    int viewportWidth = camera->getViewportWidth();
    int viewportHeight = camera->getViewportHeight();
    int cameraX = camera->getX();
    int cameraY = camera->getY();

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    std::vector<std::string> screenBuffer(viewportHeight, std::string(viewportWidth, ' '));
    std::vector<std::vector<int>> colorBuffer(viewportHeight, std::vector<int>(viewportWidth, 7));

    for (int screenY = 0; screenY < viewportHeight; screenY++) {
        int worldY = cameraY + screenY;

        if (worldY < 0 || worldY >= static_cast<int>(uiFrame.size())) {
            continue;
        }

        const std::string& worldLine = uiFrame[worldY];
        std::string& screenLine = screenBuffer[screenY];
        std::vector<int>& colorLine = colorBuffer[screenY];

        int startX = cameraX;
        int endX = min(cameraX + viewportWidth, static_cast<int>(worldLine.length()));

        if (startX < endX && startX < worldLine.length()) {
            int copyLen = min(endX - startX, viewportWidth);

            for (int i = 0; i < copyLen; i++) {
                int worldX = startX + i;
                int screenX = i;

                char c = worldLine[worldX];
                screenLine[screenX] = c;

                if (c == 'W') {
                    colorLine[screenX] = 14;
                }
            }
        }
    }

    system("cls");

    COORD coord = { 0, 0 };
    for (int y = 0; y < viewportHeight; y++) {
        coord.Y = y;
        SetConsoleCursorPosition(hConsole, coord);

        const std::string& line = screenBuffer[y];
        const std::vector<int>& colors = colorBuffer[y];

        int currentColor = -1;
        std::string currentSegment;

        for (int x = 0; x < viewportWidth; x++) {
            if (colors[x] != currentColor) {
                if (!currentSegment.empty()) {
                    SetConsoleTextAttribute(hConsole, currentColor);
                    std::cout << currentSegment;
                    currentSegment.clear();
                }
                currentColor = colors[x];
            }
            currentSegment += line[x];
        }

        if (!currentSegment.empty()) {
            SetConsoleTextAttribute(hConsole, currentColor);
            std::cout << currentSegment;
        }
    }

    SetConsoleTextAttribute(hConsole, 7);
}

//void GameEngine::renderBoss() {
//    if (bossMode && boss) {
//        boss->render();
//
//        // Рендерим корни
//        for (auto& root : bossRoots) {
//            root->render();
//        }
//    }
//}

std::string GameEngine::getVisibleSubstring(const std::string& str, int startVisualPos, int maxVisualWidth) const {
    if (startVisualPos < 0 || str.empty()) {
        return std::string(maxVisualWidth, ' ');
    }

    int startPos = min(static_cast<int>(str.length()), startVisualPos);
    int endPos = min(static_cast<int>(str.length()), startPos + maxVisualWidth);

    if (startPos >= endPos) {
        return std::string(maxVisualWidth, ' ');
    }

    std::string result = str.substr(startPos, endPos - startPos);

    if (result.length() < maxVisualWidth) {
        result += std::string(maxVisualWidth - result.length(), ' ');
    }

    return result;
}



//void GameEngine::renderBoss() {
//    if (bossMode && boss) {
//        boss->render();
//
//        // Р РµРЅРґРµСЂРёРј РєРѕСЂРЅРё
//        for (auto& root : bossRoots) {
//            root->render();
//        }
//    }
//}

void GameEngine::addEnemyBullet(std::unique_ptr<Bullet> bullet) {
    if (bullet) {
        projectiles.push_back(std::move(bullet));
    }
}

//void GameEngine::addBossRoot(std::unique_ptr<BossRoot> root) {
//    if (root) {
//        bossRoots.push_back(std::move(root));
//    }
//}

Player& GameEngine::getPlayer() {
    return *player;
}

const Player& GameEngine::getPlayer() const {
    return *player;
}

bool GameEngine::isRunning() const {
    return gameRunning;
}

int GameEngine::getScore() const {
    return score;
}

void GameEngine::setGameRunning(bool running) {
    gameRunning = running;
}

void GameEngine::updateCamera() {
    if (!player || !camera) return;

    int targetX = player->getX() - camera->getViewportWidth() / 2;
    int targetY = player->getY() - camera->getViewportHeight() / 2;

    auto& config = ConfigManager::getInstance();
    int worldWidth = config.getWorldWidth(currentLevel);
    int worldHeight = config.getWorldHeight(currentLevel);

    int maxX = max(0, worldWidth - camera->getViewportWidth());
    int maxY = max(0, worldHeight - camera->getViewportHeight());

    targetX = ourClamp(targetX, 0, maxX);
    targetY = ourClamp(targetY, 0, maxY);

    camera->setPosition(targetX, targetY);
}

void GameEngine::setDeathScreenFlag(bool flag)
{
    showDeathScreen = flag;
}

bool GameEngine::getDeathScreenFlag()
{
    return showDeathScreen;
}

int GameEngine::ourClamp(int value, int min, int max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

void GameEngine::renderGameObject(const GameObject& obj) const {
    if (!camera->isInViewport(obj.getX(), obj.getY(), obj.getWidth(), obj.getHeight())) {
        return;
    }

    int screenX = camera->worldToScreenX(obj.getX());
    int screenY = camera->worldToScreenY(obj.getY());

    GraphicsManager::renderAt(screenX, screenY, { std::string(obj.getWidth(), '=') });
}

void GameEngine::renderProjectile(const Projectile& projectile) const {
    if (!projectile.isActive() ||
        !camera->isInViewport(projectile.getX(), projectile.getY(),
            projectile.getWidth(), projectile.getHeight())) {
        return;
    }

    int screenX = camera->worldToScreenX(projectile.getX());
    int screenY = camera->worldToScreenY(projectile.getY());

    projectile.renderAt(screenX, screenY);
}

int GameEngine::getVisualLength(const std::string& str) const {
    return static_cast<int>(str.length());
}

void GameEngine::normalizeUIFrame() {
    auto& uiFrame = const_cast<std::vector<std::string>&>(GraphicsManager::getGraphic("UIFrame"));

    if (uiFrame.empty()) return;

    int maxLength = 0;
    for (const auto& line : uiFrame) {
        if (line.length() > maxLength) {
            maxLength = static_cast<int>(line.length());
        }
    }

    for (auto& line : uiFrame) {
        if (line.length() < maxLength) {
            line += std::string(maxLength - line.length(), ' ');
        }
    }
}

void GameEngine::switchLevel(const std::string& levelName) {
    Logger::Log("=== Switching from level '" + currentLevel + "' to '" + levelName + "' ===");

    int savedScore = score;

    loadLevel(levelName);

    score = savedScore;

    Logger::Log("Level switched successfully to: " + levelName);
}

int GameEngine::getVisualPosition(const std::string& str, int bytePos) const {
    int visualPos = 0;
    for (size_t i = 0; i < str.length() && i < static_cast<size_t>(bytePos); ) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        int charWidth = 1;

        if (c <= 0x7F) {
            charWidth = 1;
        }
        else if ((c & 0xE0) == 0xC0) {
            charWidth = 2;
        }
        else if ((c & 0xF0) == 0xE0) {
            charWidth = 3;
        }
        else if ((c & 0xF8) == 0xF0) {
            charWidth = 4;
        }

        i += charWidth;
        visualPos++;
    }
    return visualPos;
}

bool GameEngine::findPlayerSpawn(int& spawnX, int& spawnY) {
    const auto& uiFrame = GraphicsManager::getGraphic("UIFrame");

    for (int y = 0; y < uiFrame.size(); y++) {
        const std::string& line = uiFrame[y];
        for (int x = 0; x < line.length(); x++) {
            if (line[x] == 'P' || line[x] == 'p') {
                spawnX = x;
                spawnY = y;
                return true;
            }
        }
    }

    return false;
}

void GameEngine::removeSpawnPointFromUIFrame(int spawnX, int spawnY) {
    auto& uiFrame = const_cast<std::vector<std::string>&>(GraphicsManager::getGraphic("UIFrame"));

    if (spawnY >= 0 && spawnY < uiFrame.size() &&
        spawnX >= 0 && spawnX < uiFrame[spawnY].length()) {
        uiFrame[spawnY][spawnX] = ' ';
        Logger::Log("Removed spawn point 'P' at (" +
            std::to_string(spawnX) + ", " + std::to_string(spawnY) + ")");
    }
}

void GameEngine::createCheckpointsFromUIFrame() {
    const auto& uiFrame = GraphicsManager::getGraphic("UIFrame");
    char checkpointSymbol = ConfigManager::getInstance().getCheckpointSymbol();

    Logger::Log("Searching for checkpoints with symbol: " + std::string(1, checkpointSymbol));
    Logger::Log("UIFrame size: " + std::to_string(uiFrame.size()) + " lines");

    for (int y = 0; y < uiFrame.size(); y++) {
        const std::string& line = uiFrame[y];
        for (int x = 0; x < line.length(); x++) {
            char currentChar = line[x];

            // Используем символ из конфига для обозначения чекпоинта
            if (currentChar == checkpointSymbol) {
                auto checkpoint = std::make_shared<Checkpoint>(x, y);
                checkpoints.push_back(checkpoint);

                // Удаляем символ чекпоинта из UIFrame
                const_cast<std::string&>(line)[x] = ' ';

                Logger::Log("Checkpoint created at: (" +
                    std::to_string(x) + ", " + std::to_string(y) + ")");
            }
        }
    }
    Logger::Log("Total checkpoints created: " + std::to_string(checkpoints.size()));
}

void GameEngine::activateCheckpoint(std::shared_ptr<Checkpoint> checkpoint) {
    Logger::Log("=== activateCheckpoint called ===");

    if (checkpoint) {
        Logger::Log("Checkpoint pointer is valid at: (" +
            std::to_string(checkpoint->getX()) + ", " +
            std::to_string(checkpoint->getY()) + ")");

        if (!checkpoint->isActive()) {
            Logger::Log("Checkpoint is not active, activating...");

            // Деактивируем ВСЕ предыдущие чекпоинты
            for (auto& cp : checkpoints) {
                if (cp->isActive()) {
                    cp->setActive(false);
                    Logger::Log("Deactivated checkpoint at (" +
                        std::to_string(cp->getX()) + ", " +
                        std::to_string(cp->getY()) + ")");
                }
            }

            // Активируем новый
            checkpoint->setActive(true);
            currentCheckpoint = checkpoint;

            if (player) {
                int healthBefore = player->getHealth();
                player->heal(player->getMaxHealth());
            }

            Logger::Log("Checkpoint activated at: (" +
                std::to_string(checkpoint->getX()) + ", " +
                std::to_string(checkpoint->getY()) + ")");
            Logger::Log("Current checkpoint set to: (" +
                std::to_string(currentCheckpoint->getX()) + ", " +
                std::to_string(currentCheckpoint->getY()) + ")");
        }
        else {
            Logger::Log("Checkpoint is already active");
        }
    }
    else {
        Logger::Log("ERROR: Checkpoint pointer is null!");
    }
}

void GameEngine::respawnAtCheckpoint() {
    Logger::Log("=== respawnAtCheckpoint called ===");

    if (currentCheckpoint && player) {
        Logger::Log("Teleporting player to checkpoint: (" +
            std::to_string(currentCheckpoint->getX()) + ", " +
            std::to_string(currentCheckpoint->getY()) + ")");

        player->setX(currentCheckpoint->getX());
        player->setY(currentCheckpoint->getY());
        player->setVelocityX(0);
        player->setVelocityY(0);
        player->setOnGround(false);

        // Центрируем камеру на игроке
        if (camera) {
            camera->centerOn(player->getX(), player->getY());
            Logger::Log("Camera recentered on player");
        }
    }
    else {
        if (!currentCheckpoint) {
            Logger::Log("ERROR: currentCheckpoint is null!");
        }
        if (!player) {
            Logger::Log("ERROR: player is null!");
        }
    }
}

void GameEngine::respawnAtLevelStart() {
    if (player) {
        player->setX(levelStartX);
        player->setY(levelStartY);
        player->setVelocityX(0);
        player->setVelocityY(0);
        player->setOnGround(false);
        player->heal(player->getMaxHealth()); // Восстанавливаем все HP

        Logger::Log("Player respawned at level start: (" +
            std::to_string(levelStartX) + ", " +
            std::to_string(levelStartY) + ")");
    }
}

void GameEngine::checkWinCondition() {
    const auto& uiFrame = GraphicsManager::getGraphic("UIFrame");

    int playerX = player->getX();
    int playerY = player->getY();
    int playerWidth = player->getWidth();
    int playerHeight = player->getHeight();

    for (int py = playerY; py < playerY + playerHeight; py++) {
        if (py >= 0 && py < uiFrame.size()) {
            for (int px = playerX; px < playerX + playerWidth; px++) {
                if (px >= 0 && px < uiFrame[py].length()) {
                    if (uiFrame[py][px] == 'W') {
                        //respawnAtLevelStart(); /// YOU CAN COMMENT ON THIS

                        // TO LOAD NEXT DIALOG - SIMPLY EXIT RUNNING LOOP
                        setGameRunning(false);
                        return;
                    }
                }
            }
        }
    }
}

void GameEngine::removeWinSymbolsFromUIFrame() {
    auto& uiFrame = const_cast<std::vector<std::string>&>(GraphicsManager::getGraphic("UIFrame"));

    for (int y = 0; y < uiFrame.size(); y++) {
        std::string& line = uiFrame[y];
        for (int x = 0; x < line.length(); x++) {
            if (line[x] == 'W') {
                line[x] = ' ';
                Logger::Log("Removed win symbol 'W' at (" +
                    std::to_string(x) + ", " +
                    std::to_string(y) + ")");
            }
        }
    }
}