#include "GameEngine.h"
#include "ConfigManager.h"
#include "GraphicsManager.h"
#include "UIManager.h"
#include "Bullet.h"
#include "ParryBullet.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <windows.h>

GameEngine::GameEngine()
    : player(nullptr), bulletSpawnTimer(0), gameRunning(true), score(0),
    screenWidth(0), screenHeight(0), parryBulletSpeed(0), parryRange(5),
    currentLevel("tutorial"), bossMode(false) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

void GameEngine::initialize(const std::string& levelName) {
    ConfigManager::initialize();
    UIManager::initialize();
    loadLevel(levelName);
}

void GameEngine::loadLevel(const std::string& levelName) {
    currentLevel = levelName;
    bossMode = (levelName == "boss");

    auto& config = ConfigManager::getInstance();
    screenWidth = config.getScreenWidth();
    screenHeight = config.getScreenHeight();
    parryBulletSpeed = config.getParryBulletSpeed();
    parryRange = config.getParryRange();

    // Очищаем старые объекты
    projectiles.clear();
    platforms.clear();

    // Загружаем графику для уровня
    loadGraphicsForLevel();

    // Создаем игрока
    player = std::make_unique<Player>(
        config.getPlayerStartX(),
        config.getPlayerStartY()
    );

    // Создаем платформы для уровня
    createPlatformsFromUIFrame();

    std::cout << "Level '" << levelName << "' loaded successfully!" << std::endl;
}

void GameEngine::loadGraphicsForLevel() {
    // Базовая графика (общая для всех уровней)
    GraphicsManager::loadGraphics("graphics/player.txt", "player");
    GraphicsManager::loadGraphics("graphics/player_dodge.txt", "player_dodge");
    GraphicsManager::loadGraphics("graphics/bullet.txt", "bullet");
    GraphicsManager::loadGraphics("graphics/parry_bullet.txt", "parry_bullet");

    // Графика для конкретного уровня
    if (currentLevel == "tutorial") {
        GraphicsManager::loadGraphics("graphics/tutorial/UIFrame.txt", "UIFrame");
    }
    else if (currentLevel == "boss") {
        GraphicsManager::loadGraphics("graphics/boss/UIFrame.txt", "UIFrame");
        GraphicsManager::loadGraphics("graphics/boss/boss.txt", "boss");
    }
    else {
        // Уровень по умолчанию
        GraphicsManager::loadGraphics("graphics/UIFrame.txt", "UIFrame");
    }
}

void GameEngine::createPlatformsFromUIFrame() {
    const auto& uiFrame = GraphicsManager::getGraphic("UIFrame");

    for (int y = 0; y < uiFrame.size(); y++) {
        const std::string& line = uiFrame[y];

        int platformStart = -1;

        for (int x = 0; x < line.length(); x++) {
            if (line[x] == '=') {
                if (platformStart == -1) {
                    platformStart = x;
                }
            }
            else {
                if (platformStart != -1) {
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

    removePlatformsFromUIFrame();
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

void GameEngine::spawnBoss() {
    if (!bossMode) return;

    auto& config = ConfigManager::getInstance();
    boss = std::make_unique<Boss>(
        config.getScreenWidth() - config.getBossWidth() - 5,
        config.getScreenHeight() / 2 - config.getBossHeight() / 2,
        config.getBossWidth(),
        config.getBossHeight(),
        config.getBossHealth(),
        config.getBossPhase2HP(),
        config.getBossPhase3HP(),
        config.getBossBulletSpeed(),
        config.getBossBulletColor(),
        config.getBossBulletCooldown(),
        config.getBossRootWarningDuration(),
        config.getBossRootGrowDuration(),
        config.getBossRootDamage(),
        config.getBossRootCooldown(),
        config.getBossRootColor()
    );
}


void GameEngine::update() {
    if (!player) return;

    // Обновляем игрока
    player->update();

    // Обрабатываем коллизии
    handlePlayerCollisions();

    handlePlayerAttack();

    // Удаляем неактивные пули
    for (int i = projectiles.size() - 1; i >= 0; --i) {
        if (!projectiles[i]->isActive()) {
            projectiles.erase(projectiles.begin() + i);
        }
    }

    // Обновляем активные пули
    for (auto& bullet : projectiles) {
        if (bullet->isActive()) {
            bullet->update();

            // Удаляем пули за границами экрана
            if (bullet->getX() < 0 ||
                bullet->getX() > screenWidth + 10 ||
                bullet->getY() < 0 ||
                bullet->getY() > screenHeight) {
                bullet->setActive(false);
            }
        }
    }

    // Спавн новых пуль
    bulletSpawnTimer++;
    int randomSpawnRate;

    if (bossMode) {
        // Босс режим - чаще и сложнее
        updateBoss();
        checkBossCollisions();
        randomSpawnRate = 10 + (std::rand() % 15);
    }
    else {
        // Туториал - реже и проще
        randomSpawnRate = 20 + (std::rand() % 25);
    }

    if (bulletSpawnTimer >= randomSpawnRate) {
        spawnBullet();
        bulletSpawnTimer = 0;
    }

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
}

void GameEngine::updateBoss() {
    boss->update(*this);

    // Обновляем корни
    for (int i = bossRoots.size() - 1; i >= 0; --i) {
        bossRoots[i]->update();
        if (bossRoots[i]->getHealth() <= 0) {  // Или другой критерий удаления
            bossRoots.erase(bossRoots.begin() + i);
        }
    }
}

void GameEngine::spawnBullet() {
    bool isParryBullet = (std::rand() % 2 == 0);

    int spawnX = screenWidth;
    int spawnY;

    if (bossMode) {
        // Босс режим - пули на разных высотах
        spawnY = 5 + (std::rand() % (screenHeight - 10));
    }
    else {
        // Туториал - пули только на средних высотах
        spawnY = 8 + (std::rand() % 5);
    }

    int direction = -1;

    if (isParryBullet) {
        projectiles.push_back(std::make_unique<ParryBullet>(spawnX, spawnY, direction));
    } else {
        projectiles.push_back(std::make_unique<Bullet>(spawnX, spawnY, direction));
    }
}

void GameEngine::handlePlayerCollisions() {
    if (!player) return;

    player->setOnGround(false);
    handlePlayerWorldCollisions();
    handlePlayerPlatformCollisions();
}

void GameEngine::handlePlayerWorldCollisions() {
    if (!player) return;

    if (player->checkGroundCollision()) {
        int groundLevel = screenHeight - 10;
        player->setY(groundLevel);
        player->setVelocityY(0);
        player->setOnGround(true);
    }
}

void GameEngine::handlePlayerPlatformCollisions() {
    if (!player) return;

    for (const auto& platform : platforms) {
        bool fromTop = false;

        if (player->isCollidingWithPlatform(*platform, fromTop)) {
            if (fromTop) {
                player->setY(platform->getY() - player->getHeight());
                player->setOnGround(true);
            }
            else {
                player->setY(platform->getY() + platform->getHeight());
            }
            player->setVelocityY(0);
            break;
        }
    }
}

void GameEngine::handlePlayerAttack() {
    // Пытаемся выстрелить
    auto newBullet = player->tryFire();
    if (newBullet) {
        // Добавляем в общий массив projectiles
        projectiles.push_back(std::move(newBullet));
    }
}

void GameEngine::checkCollisions() {
    for (auto& bullet : projectiles) {
        if (!bullet->isActive()) continue;

        // Пропускаем пули игрока при проверке столкновения с самим игроком
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

void GameEngine::checkBossCollisions() {
    if (!boss || !player) return;

    // Проверяем попадание пуль игрока в босса
    for (int i = projectiles.size() - 1; i >= 0; --i) {
        if (!projectiles[i]->isActive()) continue;

        bool isPlayerBullet = (projectiles[i]->getColor() ==
            ConfigManager::getInstance().getPlayerBulletColor());

        if (isPlayerBullet && boss->checkCollision(*projectiles[i])) {
            boss->takeDamage(1);  // Каждая пуля наносит 1 урон
            projectiles[i]->setActive(false);
        }
    }

    // Проверяем столкновение игрока с корнями босса
    for (auto& root : bossRoots) {
        if (player->checkCollision(*root)) {
            player->takeDamage(root->getDamage());
            // Можно удалить корень после нанесения урона
            root->takeDamage(root->getHealth());  // Уничтожаем корень
        }
    }
}

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
    if (!player) return;

    system("cls");

    // Отрисовываем UI Frame
    GraphicsManager::renderAt(0, 0, GraphicsManager::getGraphic("UIFrame"));

    // Отрисовываем игровой UI
    UIManager::renderGameUI(*player, score);

    // Отрисовываем платформы
    for (auto& platform : platforms) {
        platform->render();
    }

    // Отрисовываем игрока
    player->render();

    // Отрисовываем пули
    for (auto& bullet : projectiles) {
        if (bullet->isActive()) {
            bullet->render();
        }
    }

    renderBoss();

    // Отрисовываем подсказки и управление
    UIManager::renderHints();
    UIManager::renderControls();

}

void GameEngine::renderBoss() {
    if (bossMode && boss) {
        boss->render();

        // Рендерим корни
        for (auto& root : bossRoots) {
            root->render();
        }
    }
}

void GameEngine::addEnemyBullet(std::unique_ptr<Bullet> bullet) {
    if (bullet) {
        projectiles.push_back(std::move(bullet));
    }
}

void GameEngine::addBossRoot(std::unique_ptr<BossRoot> root) {
    if (root) {
        bossRoots.push_back(std::move(root));
    }
}

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