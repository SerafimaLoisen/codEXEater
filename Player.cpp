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
    float newX = x + velocityX;
    float newY = y + velocityY;

    if (newX < 1) {
        newX = 1;
        velocityX = 0;
    }
    else if (newX >= worldWidth - width - 1) {
        newX = worldWidth - width - 1;
        velocityX = 0;
    }

    if (newY < 1) {
        newY = 1;
        velocityY = 0;
    }
    else if (newY >= worldHeight - height - 1) {
        newY = worldHeight - height - 1;
        velocityY = 0;
        onGround = true;
    }

    x = newX;
    y = newY;

    if (!onGround) {
        velocityY += 0.5f;
    }

    if (velocityX > 0) {
        lastDirection = 1;
    }
    else if (velocityX < 0) {
        lastDirection = -1;
    }

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
    return y >= worldHeight - height;
}

bool Player::isCollidingWithPlatform(const Platform& platform, bool& fromTop) {
    bool xCollision = (x < platform.getX() + platform.getWidth()) &&
        (x + width > platform.getX());

    if (!xCollision) return false;

    float currentBottom = y + height;
    float currentTop = y;
    float nextBottom = currentBottom + velocityY;
    float nextTop = currentTop + velocityY;

    float platformTop = platform.getY();
    float platformBottom = platform.getY() + platform.getHeight();

    if (velocityY >= 0) {
        if (currentBottom <= platformTop && nextBottom >= platformTop) {
            fromTop = true;
            return true;
        }
    }
    else if (velocityY < 0) {

        bool playerFullyBelow = (currentBottom <= platformTop);

        if (playerFullyBelow && nextTop >= platformBottom) {
            fromTop = false;
            return true;
        }
    }

    return false;
}
void Player::moveLeft() {
    velocityX = -2.0f;
}

void Player::moveRight() {
    velocityX = 2.0f;
}

void Player::stopMoving() {
    velocityX = 0;
}

void Player::jump() {
    if (onGround) {
        onGround = false;
        velocityY = -1.8f;
    }
}

void Player::startAttack() {
    isAttacking = true;
}

void Player::stopAttack() {
    isAttacking = false;
}

std::shared_ptr<Projectile> Player::tryFire() {
    if (isAttacking && fireTimer <= 0) {
        int bulletX, bulletY;

        if (lastDirection == 1) {
            bulletX = x + width;
        }
        else {
            bulletX = x - 1;
        }

        bulletY = y + height / 2;

        // Создаем пулю (используем существующий класс Bullet)
        auto bullet = std::make_shared<Bullet>(  // Но создаем Bullet
            bulletX, bulletY,
            lastDirection
        );

        bullet->setSpeed(playerBulletSpeed);
        bullet->setColor(playerBulletColor);

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

        int targetX = x + dodgeDistance;

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

bool Player::isCollidingWithSidePlatform(const SidePlatform& platform, bool& fromLeft) {
    int playerX = getX();
    int playerY = getY();
    int playerWidth = getWidth();
    int playerHeight = getHeight();

    int platformX = platform.getX();
    int platformY = platform.getY();
    int platformWidth = platform.getWidth();
    int platformHeight = platform.getHeight();

    bool yOverlap = (playerY < platformY + platformHeight) &&
        (playerY + playerHeight > platformY);

    if (!yOverlap) return false;

    bool xOverlap = (playerX < platformX + platformWidth) &&
        (playerX + playerWidth > platformX);

    if (!xOverlap) return false;


    if (velocityX > 0.1f) {
        fromLeft = true;
        return true;
    }
    else if (velocityX < -0.1f) {
        fromLeft = false;
        return true;
    }
    else {
        int playerRight = playerX + playerWidth;
        int playerLeft = playerX;
        int platformRight = platformX + platformWidth;
        int platformLeft = platformX;

        int overlapLeft = playerRight - platformLeft;
        int overlapRight = platformRight - playerLeft;

        fromLeft = (overlapLeft < overlapRight);
        return true;
    }
}