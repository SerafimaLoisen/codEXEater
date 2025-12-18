#include "GameEngine.h"

#include "ConfigManager.h"
#include "GraphicsManager.h"
#include "UIManager.h"
#include "ParryBullet.h"
#include "BossManager.h"
#include "Boss.h"
#include "Platform.h"

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <windows.h>

// =================== ctor ===================

GameEngine::GameEngine()
    : player(nullptr),
    bossManager(nullptr),
    gameRunning(true),
    score(0),
    screenWidth(0),
    screenHeight(0),
    parryBulletSpeed(0),
    parryRange(5),
    currentLevel("tutorial") {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

GameEngine::~GameEngine() = default;

// =================== init ===================

void GameEngine::initialize(const std::string& levelName) {
    ConfigManager::initialize();
    UIManager::initialize();
    loadLevel(levelName);
}

void GameEngine::loadLevel(const std::string& levelName) {
    currentLevel = levelName;

    auto& config = ConfigManager::getInstance();
    screenWidth = config.getScreenWidth();
    screenHeight = config.getScreenHeight();
    parryBulletSpeed = config.getParryBulletSpeed();
    parryRange = config.getParryRange();

    projectiles.clear();
    platforms.clear();
    bossManager.reset();

    // ===== графика =====
    GraphicsManager::loadGraphics("graphics/player.txt", "player");
    GraphicsManager::loadGraphics("graphics/player_dodge.txt", "player_dodge");
    GraphicsManager::loadGraphics("graphics/bullet.txt", "bullet");
    GraphicsManager::loadGraphics("graphics/parry_bullet.txt", "parry_bullet");

    if (levelName == "boss") {
        GraphicsManager::loadGraphics("graphics/boss/UIFrame.txt", "UIFrame");
        GraphicsManager::loadGraphics("graphics/boss/boss.txt", "boss");
        GraphicsManager::loadGraphics("graphics/boss/warning.txt", "warning");
        GraphicsManager::loadGraphics("graphics/boss/root.txt", "root");
        GraphicsManager::loadGraphics("graphics/boss/root_h.txt", "root_h");

        bossManager = std::make_unique<BossManager>(this);
    }
    else {
        GraphicsManager::loadGraphics("graphics/tutorial/UIFrame.txt", "UIFrame");
    }

    // ===== игрок =====
    player = std::make_unique<Player>(
        config.getPlayerStartX(),
        config.getPlayerStartY()
    );

    createPlatformsFromUIFrame();

    std::cout << "Level '" << levelName << "' loaded successfully!\n";
}

// =================== update ===================

void GameEngine::update() {
    if (!player) return;

    // --- игрок ---
    player->update();
    handlePlayerCollisions();
    handlePlayerAttack();

    // --- пули ---
    for (int i = static_cast<int>(projectiles.size()) - 1; i >= 0; --i) {
        auto& bullet = projectiles[i];
        if (!bullet->isActive()) {
            projectiles.erase(projectiles.begin() + i);
            continue;
        }

        bullet->update();

        // --- пули босса наносят урон игроку ---
        if (bullet->isEnemy() && player->checkCollision(*bullet)) {
            player->takeDamage(bullet->getDamage());
            bullet->setActive(false);
        }

        // --- пули игрока наносят урон боссу ---
        if (bossManager && !bullet->isEnemy()) {
            Boss& boss = bossManager->getBoss();
            if (bullet->checkCollision(boss)) {
                boss.takeDamage(bullet->getDamage());
                bullet->setActive(false);
            }
        }

        // --- проверка выхода за границы экрана ---
        if (bullet->getX() < -10 || bullet->getX() > screenWidth + 10 ||
            bullet->getY() < -10 || bullet->getY() > screenHeight + 10) {
            bullet->setActive(false);
        }
    }

    // --- босс ---
    if (bossManager) {
        bossManager->update();
        bossManager->checkPlayerCollisions(*player);
        bossManager->checkPlayerBulletCollisions(projectiles);
    }

    // --- парри ---
    handleParry();

    // --- смерть игрока ---
    if (!player->isAlive()) {
        UIManager::renderGameOver();
        Sleep(2000);
        gameRunning = false;
    }
}

// =================== render ===================

void GameEngine::render() {
    system("cls");

    GraphicsManager::renderAt(0, 0, GraphicsManager::getGraphic("UIFrame"));
    UIManager::renderGameUI(*player, score);

    for (auto& platform : platforms)
        platform->render();

    player->render();

    for (auto& bullet : projectiles) {
        if (bullet->isActive())
            bullet->render();
    }

    if (bossManager)
        bossManager->render();

    if (bossManager && currentLevel == "boss") {
        UIManager::renderBossPhase(bossManager->getBoss().getPhase());
        UIManager::renderBossHealth(bossManager->getBoss());
    }
    if (currentLevel == "tutorial") {
        UIManager::renderHints();
        UIManager::renderControls();
    }
}

// =================== player logic ===================

void GameEngine::handlePlayerAttack() {
    auto bullet = player->tryFire();
    if (bullet)
        projectiles.push_back(std::move(bullet));
}

void GameEngine::handlePlayerCollisions() {
    player->setOnGround(false);
    handlePlayerWorldCollisions();
    handlePlayerPlatformCollisions();
}

void GameEngine::handlePlayerWorldCollisions() {
    if (player->checkGroundCollision()) {
        int groundLevel = screenHeight - 10;
        player->setY(groundLevel);
        player->setVelocityY(0);
        player->setOnGround(true);
    }
}

void GameEngine::handlePlayerPlatformCollisions() {
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

// =================== parry ===================

void GameEngine::handleParry() {
    if (!player->getIsParrying()) return;

    for (auto& bullet : projectiles) {
        if (!bullet->isActive()) continue;
        if (!bullet->isEnemy()) continue;
        if (!isInParryRange(*bullet)) continue;

        auto* parryBullet = dynamic_cast<ParryBullet*>(bullet.get());
        if (!parryBullet) continue;

        // отражаем пулю
        parryBullet->parry(parryBulletSpeed * 2);

        score += 10;
    }
}

bool GameEngine::isInParryRange(const GameObject& bullet) const {
    int playerRight = player->getX() + player->getWidth();
    return bullet.getX() >= playerRight &&
        bullet.getX() <= playerRight + parryRange &&
        abs(bullet.getY() - player->getY()) <= 2;
}

// =================== platforms ===================

void GameEngine::createPlatformsFromUIFrame() {
    const auto& uiFrame = GraphicsManager::getGraphic("UIFrame");

    for (int y = 0; y < static_cast<int>(uiFrame.size()); y++) {
        const std::string& line = uiFrame[y];
        int platformStart = -1;

        for (int x = 0; x < static_cast<int>(line.length()); x++) {
            if (line[x] == '=') {
                if (platformStart == -1)
                    platformStart = x;
            }
            else if (platformStart != -1) {
                int width = x - platformStart;
                if (width > 0)
                    platforms.push_back(
                        std::make_shared<Platform>(platformStart, y, width)
                    );
                platformStart = -1;
            }
        }

        if (platformStart != -1) {
            int width = static_cast<int>(line.length()) - platformStart;
            if (width > 0)
                platforms.push_back(
                    std::make_shared<Platform>(platformStart, y, width)
                );
        }
    }

    removePlatformsFromUIFrame();
}

void GameEngine::removePlatformsFromUIFrame() {
    auto& uiFrame =
        const_cast<std::vector<std::string>&>(GraphicsManager::getGraphic("UIFrame"));

    for (auto& line : uiFrame)
        for (char& c : line)
            if (c == '=') c = ' ';
}


void GameEngine::addEnemyBullet(std::unique_ptr<Projectile> bullet) {
    if (bullet)
        projectiles.push_back(std::move(bullet));
}

Player& GameEngine::getPlayer() { return *player; }
const Player& GameEngine::getPlayer() const { return *player; }

bool GameEngine::isRunning() const { return gameRunning; }
int GameEngine::getScore() const { return score; }
void GameEngine::setGameRunning(bool running) { gameRunning = running; }
