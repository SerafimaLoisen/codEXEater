#include "Player.h"
#include "ConfigManager.h"
#include "GraphicsManager.h"
#include "Platform.h"
#include "SidePlatform.h"

Player::Player(int x, int y, int worldWidth, int worldHeight)
    : Entity(x, y,
        ConfigManager::getInstance().getPlayerWidth(),
        ConfigManager::getInstance().getPlayerHeight(),
        ConfigManager::getInstance().getPlayerHealth(), 14),
    worldWidth(worldWidth), worldHeight(worldHeight),
    screenWidth(ConfigManager::getInstance().getScreenWidth()),
    screenHeight(ConfigManager::getInstance().getScreenHeight()),
    parryDuration(ConfigManager::getInstance().getParryDuration()),
    dodgeDuration(ConfigManager::getInstance().getDodgeDuration()),
    dodgeDistance(ConfigManager::getInstance().getDodgeDistance()),
    velocityX(0), velocityY(0), onGround(true),
    isParrying(false), isDodging(false),
    parryTimer(0), dodgeTimer(0),
    fireTimer(0),
    lastDirection(1),
    isAttacking(false),
    playerBulletSpeed(ConfigManager::getInstance().getPlayerBulletSpeed()),
    playerFireRate(ConfigManager::getInstance().getPlayerFireRate()),
    playerBulletColor(ConfigManager::getInstance().getPlayerBulletColor()) {
}

void Player::update() {
    // ОБНОВЛЯЕМ ПОЗИЦИЮ ПЕРВОЙ
    // Рассчитываем новую позицию
    float newX = x + velocityX;
    float newY = y + velocityY;

    // ЖЕСТКАЯ ПРОВЕРКА ГРАНИЦ МИРА ПЕРЕД ОБНОВЛЕНИЕМ
    // Левая граница
    if (newX < 1) {
        newX = 1;
        velocityX = 0;
    }
    // Правая граница (учитываем ширину игрока и границу справа = 1)
    else if (newX >= worldWidth - width - 1) {
        newX = worldWidth - width - 1;
        velocityX = 0;
    }

    // Верхняя граница (граница = 1)
    if (newY < 1) {
        newY = 1;
        velocityY = 0;
    }
    else if (newY >= worldHeight - height - 1) {
        newY = worldHeight - height - 1;
        velocityY = 0;
        onGround = true;  // Игрок стоит на "земле" мира
    }

    // Обновляем позицию
    x = newX;
    y = newY;

    // ФИЗИКА ПОСЛЕ ОБНОВЛЕНИЯ ПОЗИЦИИ
    if (!onGround) {
        velocityY += 0.5f; // Гравитация
    }

    // Обновляем направление
    if (velocityX > 0) {
        lastDirection = 1;
    }
    else if (velocityX < 0) {
        lastDirection = -1;
    }

    // Таймеры способностей
    if (isParrying) parryTimer--;
    if (isDodging) dodgeTimer--;
    if (parryTimer <= 0) isParrying = false;
    if (dodgeTimer <= 0) isDodging = false;
    if (isAttacking && fireTimer > 0) {
        fireTimer--;
    }
}


void Player::render() {
    static const auto& playerSprite = GraphicsManager::getGraphic("player");
    static const auto& playerDodgeSprite = GraphicsManager::getGraphic("player_dodge");

    if (isDodging) {
        GraphicsManager::renderAt(x, y, playerDodgeSprite);
    }
    else {
        GraphicsManager::renderAt(x, y, playerSprite, 14);
    }
}

bool Player::checkGroundCollision() const {
    // Проверяем столкновение с нижней границей мира
    // Добавляем небольшую погрешность для корректной работы
    return y >= worldHeight - height;
}

bool Player::isCollidingWithPlatform(const Platform& platform, bool& fromTop) {
    // Проверяем пересечение по X
    bool xCollision = (x < platform.getX() + platform.getWidth()) &&
        (x + width > platform.getX());

    if (!xCollision) return false;

    // Получаем текущую и следующую позицию игрока
    float currentBottom = y + height;
    float currentTop = y;
    float nextBottom = currentBottom + velocityY;
    float nextTop = currentTop + velocityY;

    float platformTop = platform.getY();
    float platformBottom = platform.getY() + platform.getHeight();

    // Проверяем столкновение сверху (падаем на платформу)
    if (velocityY >= 0) {
        // Игрок падает вниз
        if (currentBottom <= platformTop && nextBottom >= platformTop) {
            fromTop = true;
            return true;
        }
    }
    // Проверяем столкновение снизу (прыгаем в платформу)
    else if (velocityY < 0) {
        // Игрок движется вверх

        // Проверяем, полностью ли игрок под платформой
        bool playerFullyBelow = (currentBottom <= platformTop);

        // Если игрок полностью под платформой и продолжает движение вверх
        if (playerFullyBelow && nextTop >= platformBottom) {
            fromTop = false;
            return true;
        }
    }

    return false;
}
void Player::moveLeft() {
    velocityX = -3.0f;
}

void Player::moveRight() {
    velocityX = 3.0f;
}

void Player::stopMoving() {
    velocityX = 0;
}

void Player::jump() {
    if (onGround) {
        onGround = false;
        velocityY = -2.0f;
    }
}

void Player::startAttack() {
    isAttacking = true;
}

void Player::stopAttack() {
    isAttacking = false;
}

std::unique_ptr<Projectile> Player::tryFire() {
    if (isAttacking && fireTimer <= 0) {
        // Рассчитываем позицию выстрела
        int bulletX, bulletY;

        if (lastDirection == 1) {  // Стреляем вправо
            bulletX = x + width;  // Справа от игрока
        }
        else {  // Стреляем влево
            bulletX = x - 1;  // Слева от игрока
        }

        bulletY = y + height / 2;  // Центр по вертикали

        // Создаем пулю (используем существующий класс Bullet)
        auto bullet = std::make_unique<Bullet>(  // Но создаем Bullet
            bulletX, bulletY,
            lastDirection
        );

        // Настраиваем специфичные для игрока параметры
        bullet->setSpeed(playerBulletSpeed);
        bullet->setColor(playerBulletColor);

        // Сбрасываем таймер
        fireTimer = playerFireRate;

        return bullet;
    }

    return nullptr;
}

void Player::startParry() {
    if (!isParrying) {
        isParrying = true;
        parryTimer = parryDuration;
    }
}

void Player::startDodge() {
    if (!isDodging) {
        isDodging = true;
        dodgeTimer = dodgeDuration;

        // Рассчитываем новую позицию с учетом границ
        int targetX = x + dodgeDistance;

        // Проверяем границы мира с учетом отступов
        if (targetX < 1) {
            x = 1;
        }
        else if (targetX > worldWidth - width - 1) {
            x = worldWidth - width - 1;
        }
        else {
            x = targetX;
        }
    }
}

void Player::renderAt(int screenX, int screenY) const {
    static const auto& playerSprite = GraphicsManager::getGraphic("player");
    static const auto& playerDodgeSprite = GraphicsManager::getGraphic("player_dodge");

    if (isDodging) {
        GraphicsManager::renderAt(screenX, screenY, playerDodgeSprite);
    }
    else {
        GraphicsManager::renderAt(screenX, screenY, playerSprite, 14);
    }
}

// Player.cpp - добавляем метод
bool Player::isCollidingWithSidePlatform(const SidePlatform& platform, bool& fromLeft) {
    // Текущая позиция игрока
    int playerX = getX();
    int playerY = getY();
    int playerWidth = getWidth();
    int playerHeight = getHeight();

    // Позиция платформы
    int platformX = platform.getX();
    int platformY = platform.getY();
    int platformWidth = platform.getWidth();
    int platformHeight = platform.getHeight();

    // Проверяем пересечение по Y
    bool yOverlap = (playerY < platformY + platformHeight) &&
        (playerY + playerHeight > platformY);

    if (!yOverlap) return false;

    // Проверяем пересечение по X на текущей позиции
    bool xOverlap = (playerX < platformX + platformWidth) &&
        (playerX + playerWidth > platformX);

    if (!xOverlap) return false;

    // ОПРЕДЕЛЯЕМ СТОРОНУ СТОЛКНОВЕНИЯ ПРОСТО:
    // Если игрок ДВИЖЕТСЯ ВПРАВО, значит он столкнулся с ЛЕВОЙ стороной платформы
    // Если игрок ДВИЖЕТСЯ ВЛЕВО, значит он столкнулся с ПРАВОЙ стороной платформы

    if (velocityX > 0.1f) {
        // Движется вправо -> столкновение с левой стороной платформы
        fromLeft = true;
        return true;
    }
    else if (velocityX < -0.1f) {
        // Движется влево -> столкновение с правой стороной платформы
        fromLeft = false;
        return true;
    }
    else {
        // Если скорость почти 0, проверяем текущее перекрытие
        // Определяем, с какой стороны игрок ближе к платформе
        int playerRight = playerX + playerWidth;
        int playerLeft = playerX;
        int platformRight = platformX + platformWidth;
        int platformLeft = platformX;

        int overlapLeft = playerRight - platformLeft;
        int overlapRight = platformRight - playerLeft;

        // Ближайшая сторона определяет направление столкновения
        fromLeft = (overlapLeft < overlapRight);
        return true;
    }
}