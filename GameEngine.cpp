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

GameEngine::GameEngine()
    : player(nullptr), bulletSpawnTimer(0), gameRunning(true), score(0),
    screenWidth(0), screenHeight(0), parryBulletSpeed(0), parryRange(5),
    currentLevel("tutorial"), bossMode(false), camera(nullptr) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
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

    // Загружаем графику для уровня
    loadGraphicsForLevel();
    normalizeUIFrame();

    // Рассчитываем размер мира автоматически ТОЛЬКО из файла уровня
    int worldWidth = config.getWorldWidth(levelName);
    int worldHeight = config.getWorldHeight(levelName);

    Logger::Log("World size calculated: " + std::to_string(worldWidth) + "x" + std::to_string(worldHeight));

    // Создаем камеру
    camera = std::make_unique<Camera>(
        config.getViewportWidth(),
        config.getViewportHeight(),
        worldWidth,
        worldHeight
    );

    Logger::Log("Camera created: viewport=" +
        std::to_string(config.getViewportWidth()) + "x" +
        std::to_string(config.getViewportHeight()));

    // Очищаем старые объекты
    projectiles.clear();
    platforms.clear();
    sidePlatforms.clear();

    // Создаем платформы для уровня
    createPlatformsFromUIFrame();

    // НОВОЕ: Ищем позицию спавна игрока (буква 'P') в UIFrame
    int spawnX = -1;
    int spawnY = -1;
    bool foundSpawnPoint = findPlayerSpawn(spawnX, spawnY);

    if (!foundSpawnPoint) {
        // Если не нашли 'P', используем позицию из конфига
        spawnX = config.getPlayerStartX();
        spawnY = config.getPlayerStartY();
        Logger::Log("Player spawn 'P' not found, using config: (" +
            std::to_string(spawnX) + ", " + std::to_string(spawnY) + ")");
    }
    else {
        Logger::Log("Player spawn found at 'P': (" +
            std::to_string(spawnX) + ", " + std::to_string(spawnY) + ")");

        // Удаляем букву 'P' из UIFrame, чтобы она не отображалась
        removeSpawnPointFromUIFrame(spawnX, spawnY);
    }

    // Пытаемся найти лучшее место (на платформе) если точка спавна не указана через 'P'
    const auto& uiFrame = GraphicsManager::getGraphic("UIFrame");
    if (!foundSpawnPoint && !uiFrame.empty()) {
        // Ищем платформу под предполагаемой позицией
        bool foundSafeSpot = false;
        for (int y = worldHeight - 5; y > 0 && !foundSafeSpot; y--) {
            if (y < uiFrame.size()) {
                for (int x = spawnX - 10; x < spawnX + 10 && x < uiFrame[y].length(); x++) {
                    if (x >= 0 && uiFrame[y][x] == '=') {
                        // Нашли платформу, ставим игрока над ней
                        spawnX = x;
                        spawnY = y - 5;
                        foundSafeSpot = true;
                        break;
                    }
                }
            }
        }
    }

    // Создаем игрока
    player = std::make_unique<Player>(spawnX, spawnY, worldWidth, worldHeight);

    Logger::Log("Player created at world coords: (" +
        std::to_string(spawnX) + ", " + std::to_string(spawnY) + ")");

    // Центрируем камеру на игроке
    camera->centerOn(player->getX(), player->getY());

    Logger::Log("Camera centered on player: camera(" +
        std::to_string(camera->getX()) + ", " + std::to_string(camera->getY()) + ")");

    std::cout << "=== Level '" << levelName << "' loaded ===" << std::endl;
    std::cout << "World size: " << worldWidth << "x" << worldHeight << std::endl;
    std::cout << "Viewport: " << camera->getViewportWidth() << "x" << camera->getViewportHeight() << std::endl;
    std::cout << "Player start: (" << spawnX << ", " << spawnY << ")" << std::endl;
}

void GameEngine::loadGraphicsForLevel() {
    // Базовая графика (общая для всех уровней)
    GraphicsManager::loadGraphics("graphics/player.txt", "player");
    GraphicsManager::loadGraphics("graphics/player_dodge.txt", "player_dodge");
    GraphicsManager::loadGraphics("graphics/bullet.txt", "bullet");
    GraphicsManager::loadGraphics("graphics/parry_bullet.txt", "parry_bullet");

    // Определяем путь к графике уровня
    std::string levelFolder;
    if (currentLevel == "boss") {
        GraphicsManager::loadGraphics("graphics/boss/UIFrame.txt", "UIFrame");
    }
    else {
        GraphicsManager::loadGraphics("graphics/levels/" + currentLevel + ".txt", "UIFrame");
    }

    // Если есть босс - загружаем его графику
    if (currentLevel == "boss") {
        GraphicsManager::loadGraphics("graphics/boss/boss.txt", "boss");
    }
}

// GameEngine.cpp - обновляем метод createPlatformsFromUIFrame
void GameEngine::createPlatformsFromUIFrame() {
    const auto& uiFrame = GraphicsManager::getGraphic("UIFrame");

    for (int y = 0; y < uiFrame.size(); y++) {
        const std::string& line = uiFrame[y];

        // Для горизонтальных платформ (=)
        int platformStart = -1;

        for (int x = 0; x < line.length(); x++) {
            char currentChar = line[x];

            // ИГНОРИРУЕМ букву 'P' - это точка спавна игрока
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
                // Создаем вертикальную платформу
                // Находим высоту вертикальной платформы
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

                    // Пропускаем уже обработанные символы '#' по вертикали
                    platformStart = -1;
                    // Пропускаем этот столбец
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

    // 2. ПОТОМ обновляем позицию игрока (учитывая уже скорректированные скорости)
    player->update();

    // 3. Обновляем камеру
    updateCamera();

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

    //player->setOnGround(false);
    handlePlayerWorldCollisions();
    handlePlayerPlatformCollisions();
    handlePlayerSidePlatformCollisions(); // Новый метод
}

// GameEngine.cpp - обновленный метод
void GameEngine::handlePlayerSidePlatformCollisions() {
    if (!player) return;

    // Только если игрок движется (горизонтальная скорость)
    if (fabs(player->getVelocityX()) < 0.1f) return;

    // Берем ТЕКУЩУЮ позицию игрока
    int currentX = player->getX();
    int currentY = player->getY();
    int playerWidth = player->getWidth();
    float velocityX = player->getVelocityX();

    // Рассчитываем ПРЕДПОЛАГАЕМУЮ позицию на основе скорости
    int predictedX = currentX + static_cast<int>(velocityX);

    bool collisionDetected = false;

    for (const auto& sidePlatform : sidePlatforms) {
        bool fromLeft = false;

        // ВРЕМЕННО перемещаем игрока в предполагаемую позицию для проверки
        player->setX(predictedX);

        if (player->isCollidingWithSidePlatform(*sidePlatform, fromLeft)) {
            collisionDetected = true;

            // ВОЗВРАЩАЕМ на исходную позицию
            player->setX(currentX);

            if (fromLeft) {
                // Столкновение СЛЕВА (игрок движется вправо, встречает левую сторону платформы)
                // Ставим игрока СЛЕВА от платформы
                player->setX(sidePlatform->getX() - playerWidth);
            }
            else {
                // Столкновение СПРАВА (игрок движется влево, встречает правую сторону платформы)
                // Ставим игрока СПРАВА от платформы
                player->setX(sidePlatform->getX() + sidePlatform->getWidth());
            }

            // Останавливаем горизонтальное движение
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

        // Возвращаем исходную позицию перед следующей проверкой
        player->setX(currentX);
    }

    // Если коллизии не было, оставляем исходную позицию
    // Фактическое движение будет обработано в player->update()
}

void GameEngine::handlePlayerWorldCollisions() {
    if (!player) return;

    auto& config = ConfigManager::getInstance();
    int worldHeight = config.getWorldHeight(currentLevel);

    if (player->checkGroundCollision()) {
        // Используем мировую высоту
        player->setY(worldHeight - player->getHeight());
        player->setVelocityY(0);
        player->setOnGround(true);  // Устанавливаем onGround ТОЛЬКО если действительно на земле
    }
    else {
        player->setOnGround(false); // Если не на земле
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
                player->setOnGround(true);  // На платформе
                foundPlatform = true;
            }
            else {
                player->setY(platform->getY() + platform->getHeight());
            }
            player->setVelocityY(0);
            break;
        }
    }

    // Если не нашли платформу под ногами и не на земле мира
    if (!foundPlatform && !player->checkGroundCollision()) {
        player->setOnGround(false);
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

    // Отрисовываем видимую часть UI Frame с учетом камеры
    renderUIFrameWithCamera();

    // Отрисовываем только игрока и пули поверх UI Frame
    // НЕ отрисовываем платформы отдельно - они уже в UI Frame

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

    // Используем двойную буферизацию
    std::vector<std::string> screenBuffer(viewportHeight, std::string(viewportWidth, ' '));

    // Рендерим только видимую часть UI Frame
    for (int screenY = 0; screenY < viewportHeight; screenY++) {
        int worldY = cameraY + screenY;

        if (worldY < 0 || worldY >= static_cast<int>(uiFrame.size())) {
            continue;
        }

        const std::string& worldLine = uiFrame[worldY];
        std::string& screenLine = screenBuffer[screenY];

        // Копируем видимую часть строки
        int startX = cameraX;
        int endX = min(cameraX + viewportWidth, static_cast<int>(worldLine.length()));

        if (startX < endX && startX < worldLine.length()) {
            int copyLen = min(endX - startX, viewportWidth);
            screenLine.replace(0, copyLen, worldLine.substr(startX, copyLen));
        }
    }

    // Выводим весь буфер на экран за один раз
    system("cls");

    COORD coord = { 0, 0 };
    for (int y = 0; y < viewportHeight; y++) {
        coord.Y = y;
        SetConsoleCursorPosition(hConsole, coord);
        std::cout << screenBuffer[y];
    }
}

std::string GameEngine::getVisibleSubstring(const std::string& str, int startVisualPos, int maxVisualWidth) const {
    if (startVisualPos < 0 || str.empty()) {
        return std::string(maxVisualWidth, ' ');
    }

    // Просто обрезаем строку для ASCII
    int startPos = min(static_cast<int>(str.length()), startVisualPos);
    int endPos = min(static_cast<int>(str.length()), startPos + maxVisualWidth);

    if (startPos >= endPos) {
        return std::string(maxVisualWidth, ' ');
    }

    std::string result = str.substr(startPos, endPos - startPos);

    // Дополняем пробелами если нужно
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

    // Проверяем границы
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
        return; // Не рендерим объекты вне видимой области
    }

    int screenX = camera->worldToScreenX(obj.getX());
    int screenY = camera->worldToScreenY(obj.getY());

    // Здесь нужно будет адаптировать рендеринг под вашу систему
    // Например, для платформ:
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

    // Используйте существующий рендеринг пуль с учетом камеры
    projectile.renderAt(screenX, screenY);
}

int GameEngine::getVisualLength(const std::string& str) const {
    // Для простоты считаем, что все символы в UI Frame - ASCII или имеют визуальную длину 1
    return static_cast<int>(str.length());
}

void GameEngine::normalizeUIFrame() {
    auto& uiFrame = const_cast<std::vector<std::string>&>(GraphicsManager::getGraphic("UIFrame"));

    if (uiFrame.empty()) return;

    // Находим максимальную длину строки
    int maxLength = 0;
    for (const auto& line : uiFrame) {
        if (line.length() > maxLength) {
            maxLength = static_cast<int>(line.length());
        }
    }

    // Нормализуем все строки
    for (auto& line : uiFrame) {
        if (line.length() < maxLength) {
            line += std::string(maxLength - line.length(), ' ');
        }
    }
}

void GameEngine::switchLevel(const std::string& levelName) {
    Logger::Log("=== Switching from level '" + currentLevel + "' to '" + levelName + "' ===");

    // Сохраняем состояние игрока (здоровье, бонусы)
    int savedHealth = player ? player->getHealth() : ConfigManager::getInstance().getPlayerHealth();
    int savedScore = score;

    // Загружаем новый уровень
    loadLevel(levelName);

    if (player) {
        player->heal(player->getMaxHealth());
    }

    // Сохраняем или сбрасываем счет
    // score = savedScore; // Сохраняем счет между уровнями
    score = 0; // Или сбрасываем счет при переходе

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
        uiFrame[spawnY][spawnX] = ' '; // Заменяем 'P' на пробел
        Logger::Log("Removed spawn point 'P' at (" +
            std::to_string(spawnX) + ", " + std::to_string(spawnY) + ")");
    }
}