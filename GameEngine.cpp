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
#include <algorithm>
#pragma endregion


GameEngine* GameEngine::instance = nullptr;

GameEngine* GameEngine::getInstance() {
    return instance;
}

GameEngine::GameEngine()
    : player(nullptr), bulletSpawnTimer(0), gameRunning(true), score(0),
    screenWidth(0), screenHeight(0), parryBulletSpeed(0), parryRange(5),
    currentLevel("tutorial"), bossMode(false), camera(nullptr) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    instance = this;
}

void GameEngine::initialize(const std::string& levelName) {
    ConfigManager::initialize();
    UIManager::initialize();
    loadLevel(levelName);
}

void GameEngine::loadLevel(const std::string& levelName) {
    Logger::Log("=== GameEngine::loadLevel('" + levelName + "') ===");

    currentLevel = levelName;
    bossMode = (levelName == "boss");

    auto& config = ConfigManager::getInstance();
    screenWidth = config.getScreenWidth();
    screenHeight = config.getScreenHeight();
    parryBulletSpeed = config.getParryBulletSpeed();
    parryRange = config.getParryRange();

    Logger::Log("Config loaded: screen=" + std::to_string(screenWidth) + "x" + std::to_string(screenHeight));

    loadGraphicsForLevel();
    normalizeUIFrame();

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

    createPlatformsFromUIFrame();

#pragma region CREATE_ENEMIES
    DefineGlobalHostileEntitiesPrefabs();
    // ������ ������ ��� ������
    CreateHostileEntitiesFromLevelMap(DefineLevelBasedHostileEntitiesPrefabs(0));
#pragma endregion


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

    player = std::make_unique<Player>(spawnX, spawnY, worldWidth, worldHeight);

    Logger::Log("Player created at world coords: (" +
        std::to_string(spawnX) + ", " + std::to_string(spawnY) + ")");

    camera->centerOn(player->getX(), player->getY());

    Logger::Log("Camera centered on player: camera(" +
        std::to_string(camera->getX()) + ", " + std::to_string(camera->getY()) + ")");

    std::cout << "=== Level '" << levelName << "' loaded ===" << std::endl;
    std::cout << "World size: " << worldWidth << "x" << worldHeight << std::endl;
    std::cout << "Viewport: " << camera->getViewportWidth() << "x" << camera->getViewportHeight() << std::endl;
    std::cout << "Player start: (" << spawnX << ", " << spawnY << ")" << std::endl;
	
}

void GameEngine::loadGraphicsForLevel() {
    GraphicsManager::loadGraphics("graphics/player.txt", "player");
    GraphicsManager::loadGraphics("graphics/player_dodge.txt", "player_dodge");
    GraphicsManager::loadGraphics("graphics/bullet.txt", "bullet");
    GraphicsManager::loadGraphics("graphics/parry_bullet.txt", "parry_bullet");

#pragma region LOAD_HOSTILE_SPRITES
    GraphicsManager::loadGraphics("graphics/hostile/enemy_default.txt", "enemy_default");
#pragma endregion

    // ������� ��� ����������� ������
    if (currentLevel == "boss") {
        GraphicsManager::loadGraphics("graphics/boss/UIFrame.txt", "UIFrame");
    }
    else {
        GraphicsManager::loadGraphics("graphics/levels/" + currentLevel + ".txt", "UIFrame");
    }
}

#pragma region DEFINE_HOSTILE_ENTITIES

void GameEngine::DefineGlobalHostileEntitiesPrefabs() {
    
    // COMPONENTS CONFIGS

    DealDamageOnOverlapComponentConfig instantKillOnOverlap = DealDamageOnOverlapComponentConfig(10, 0);

    // PREFABS
    
    CBE spikeUp = std::make_shared<ComponentsBasedEntity>(20, 5, 2, 2, 5);
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
    
    // Movement on Axis X (to the left and to the right) with Speed of 1, Start Direction of 1 (to the right) and with the Rest Time of 4
    FollowLineTrajectoryComponentConfig horizontalMovementBy8Units = FollowLineTrajectoryComponentConfig({ 8, 8 }, 1, 1, Axis::X, 4);
    
    // 
    DealDamageOnOverlapComponentConfig defaultOverlapDamage = DealDamageOnOverlapComponentConfig();
    
    // Emit in the -1 direction on Axis X with Projectile Speed of 2, 3 Projectiles in a Row with Time Between Emissions=3 and Time Between Sequences of Emissions=6
    EmitProjectilesComponentConfig emitToTheLeftWithSpeedOf2 = EmitProjectilesComponentConfig({ -1, 0 }, 2, 3, 3, 6);

    std::shared_ptr<HostileEntitiesPrefabs> prefabs = std::make_shared<HostileEntitiesPrefabs>();

    switch (levelIndex) {
        case 0:
        {
            // LEVEL BASED PREFABS

            // TEST ENEMY PREFAB
            std::shared_ptr<ComponentsBasedEntity> testEnemy = std::make_shared<ComponentsBasedEntity>(20, 5, 5, 3, 5);
            testEnemy->AddComponent(*new FollowLineTrajectoryComponent(testEnemy.get(), horizontalMovementBy8Units));

            // ANOTHER TEST ENEMY PREFAB
            std::shared_ptr<ComponentsBasedEntity> anotherTestEnemy = std::make_shared<ComponentsBasedEntity>(20, 5, 5, 3, 5, true);
            anotherTestEnemy->AddComponent(*new FollowLineTrajectoryComponent(anotherTestEnemy.get(), horizontalMovementBy8Units));
            anotherTestEnemy->AddComponent(*new DealDamageOnOverlapComponent(anotherTestEnemy.get(), *getPlayerPtr(), defaultOverlapDamage));
            anotherTestEnemy->AddComponent(*new TakeDamageOnOverlapComponent(anotherTestEnemy.get(), playerProjectiles, 1));

            // PROJECTILES EMITTER PREFAB
            std::shared_ptr<ComponentsBasedEntity> projectilesEmitter = std::make_shared<ComponentsBasedEntity>(20, 5, 1, 1, 5, false, true, std::vector<std::string>{" "});
            projectilesEmitter->AddComponent(*new EmitProjectilesComponent(projectilesEmitter.get(), emitToTheLeftWithSpeedOf2, projectiles, getPlayerPtr()));

            // BIND SYMBOL_ID TO PREFAB

            (*prefabs)["p"] = testEnemy;
            (*prefabs)["p2"] = testEnemy;
            (*prefabs)["p3"] = anotherTestEnemy;
            
            (*prefabs)["s"] = projectilesEmitter;
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

    handlePlayerCollisions();

    player->update();

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

    // Проверяем условие завершения игры
    if (!player->isAlive()) {
        UIManager::renderGameOver();
        Sleep(2000);
        gameRunning = false;
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
        }
        
    }  
#pragma endregion
}

//void GameEngine::updateBoss() {
//    boss->update(*this);
//
//    // Обновляем корни
//    for (int i = bossRoots.size() - 1; i >= 0; --i) {
//        bossRoots[i]->update();
//        if (bossRoots[i]->getHealth() <= 0) {  // Или другой критерий удаления
//            bossRoots.erase(bossRoots.begin() + i);
//        }
//    }
//}

void GameEngine::spawnBullet() {
    bool isParryBullet = (std::rand() % 2 == 0);

    // Используем локально рассчитанные размеры мира
    auto& config = ConfigManager::getInstance();
    int worldWidth = config.getWorldWidth(currentLevel);
    int worldHeight = config.getWorldHeight(currentLevel);

    int spawnX = worldWidth;  // Справа от мира
    int spawnY;

    if (bossMode) {
        // Босс режим - пули на разных высотах
        spawnY = 5 + (std::rand() % (worldHeight - 10));
    }
    else {
        // Туториал - пули только на средних высотах
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
		
        // ��������� � ����� ������ projectiles
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

            player->takeDamage(1);
            bullet->setActive(false);
        }
    }
}

//void GameEngine::checkBossCollisions() {
//    if (!boss || !player) return;
//
//    // Проверяем попадание пуль игрока в босса
//    for (int i = projectiles.size() - 1; i >= 0; --i) {
//        if (!projectiles[i]->isActive()) continue;
//
//        bool isPlayerBullet = (projectiles[i]->getColor() ==
//            ConfigManager::getInstance().getPlayerBulletColor());
//
//        if (isPlayerBullet && boss->checkCollision(*projectiles[i])) {
//            boss->takeDamage(1);  // Каждая пуля наносит 1 урон
//            projectiles[i]->setActive(false);
//        }
//    }
//
//    // Проверяем столкновение игрока с корнями босса
//    for (auto& root : bossRoots) {
//        if (player->checkCollision(*root)) {
//            player->takeDamage(root->getDamage());
//            // Можно удалить корень после нанесения урона
//            root->takeDamage(root->getHealth());  // Уничтожаем корень
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

    for (int screenY = 0; screenY < viewportHeight; screenY++) {
        int worldY = cameraY + screenY;

        if (worldY < 0 || worldY >= static_cast<int>(uiFrame.size())) {
            continue;
        }

        const std::string& worldLine = uiFrame[worldY];
        std::string& screenLine = screenBuffer[screenY];

        int startX = cameraX;
        int endX = min(cameraX + viewportWidth, static_cast<int>(worldLine.length()));

        if (startX < endX && startX < worldLine.length()) {
            int copyLen = min(endX - startX, viewportWidth);
            screenLine.replace(0, copyLen, worldLine.substr(startX, copyLen));
        }
    }

    system("cls");



	// POTENTIAL PROBLEMS START HERE
    COORD coord = { 0, 0 };
    for (int y = 0; y < viewportHeight; y++) {
        coord.Y = y;
        SetConsoleCursorPosition(hConsole, coord);
        std::cout << screenBuffer[y];
	// AND END HERE
		
    // ������������ UI Frame
    GraphicsManager::renderAt(0, 0, GraphicsManager::getGraphic("UIFrame"));

    // ������������ ������� UI
    UIManager::renderGameUI(*player, score);

    // ������������ ���������
    for (auto& platform : platforms) {
        platform->render();
    }

    // ������������ ������
    player->render();

    // ������������ ����
    for (auto& bullet : projectiles) {
        if (bullet->isActive()) {
            bullet->render();
        }
    }

#pragma region RENDER_ENEMIES
    for (auto& enemy : hostileEntitiesToProcess) {
        enemy->render();
    }
#pragma endregion

    renderBoss();

    // ������������ ��������� � ����������
    UIManager::renderHints();
    UIManager::renderControls();

}

void GameEngine::renderBoss() {
    if (bossMode && boss) {
        boss->render();

        // �������� �����
        for (auto& root : bossRoots) {
            root->render();
        }
    }
}

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
//        // Рендерим корни
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

    targetX = std::clamp(targetX, 0, maxX);
    targetY = std::clamp(targetY, 0, maxY);

    camera->setPosition(targetX, targetY);
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

    int savedHealth = player ? player->getHealth() : ConfigManager::getInstance().getPlayerHealth();
    int savedScore = score;

    loadLevel(levelName);

    if (player) {
        player->heal(player->getMaxHealth());
    }

    score = 0;

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