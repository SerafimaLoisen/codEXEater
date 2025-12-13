#include "Player.h"
#include "ConfigManager.h"
#include "GraphicsManager.h"
#include "Platform.h"

Player::Player(int x, int y)
    : Entity(x, y,
        ConfigManager::getInstance().getPlayerWidth(),
        ConfigManager::getInstance().getPlayerHeight(),
        ConfigManager::getInstance().getPlayerHealth(), 14),
    screenWidth(ConfigManager::getInstance().getScreenWidth()),
    screenHeight(ConfigManager::getInstance().getScreenHeight()),
    parryDuration(ConfigManager::getInstance().getParryDuration()),
    dodgeDuration(ConfigManager::getInstance().getDodgeDuration()),
    dodgeDistance(ConfigManager::getInstance().getDodgeDistance()),
    velocityX(0), velocityY(0), onGround(true),
    isParrying(false), isDodging(false),
    parryTimer(0), dodgeTimer(0),
    fireTimer(0),
    lastDirection(1),  // По умолчанию смотрим вправо
    isAttacking(false),
    playerBulletSpeed(ConfigManager::getInstance().getPlayerBulletSpeed()),
    playerFireRate(ConfigManager::getInstance().getPlayerFireRate()),
    playerBulletColor(ConfigManager::getInstance().getPlayerBulletColor()) {
}

void Player::update() {
    // Физика
    if (!onGround) {
        velocityY += 0.5f;
        y += velocityY;
    }

    // Движение
    x += velocityX;
    if (velocityX > 0) {
        lastDirection = 1;  // Движемся вправо
    }
    else if (velocityX < 0) {
        lastDirection = -1; // Движемся влево
    }

    // Границы
    if (x < 1) x = 1;
    if (x > screenWidth - width - 1) x = screenWidth - width - 1;
    if (y < 1) y = 1;

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
    
    int groundLevel = screenHeight - 10;
    return y >= groundLevel;
}

bool Player::isCollidingWithPlatform(const Platform& platform, bool& fromTop) {
    // Проверяем пересечение по X
    bool xCollision = (x < platform.getX() + platform.getWidth()) &&
        (x + width > platform.getX());

    if (!xCollision) return false;

    // Проверяем столкновение сверху (падаем на платформу)
    if (velocityY >= 0 &&
        y + height <= platform.getY() &&
        y + height + velocityY >= platform.getY()) {
        fromTop = true;
        return true;
    }

    // Проверяем столкновение снизу (прыгаем в платформу)
    if (velocityY < 0 &&
        y >= platform.getY() + platform.getHeight() &&
        y + velocityY <= platform.getY() + platform.getHeight()) {
        fromTop = false;
        return true;
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
        x += dodgeDistance;

        // Проверка границ после уворота
        if (x < 1) x = 1;
        if (x > screenWidth - width - 1) x = screenWidth - width - 1;
    }
}