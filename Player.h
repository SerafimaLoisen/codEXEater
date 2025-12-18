#pragma once
#include "Entity.h"
#include "Bullet.h"
#include <memory> 

class Platform;

class Player : public Entity {
private:
    float velocityX, velocityY;
    bool onGround;
    bool isParrying, isDodging;
    int parryTimer, dodgeTimer;

    int screenWidth, screenHeight;
    int parryDuration, dodgeDuration, dodgeDistance;

    // Атака
    int fireTimer;
    int lastDirection;  // 1 = вправо, -1 = влево
    bool isAttacking;

    // Настройки атаки из конфига
    int playerBulletColor;
    int bulletSpeed;
    int playerCooldown;

public:
    Player(int x, int y);

    void update() override;
    void render() override;

    bool isCollidingWithPlatform(const Platform& platform, bool& fromTop);
    bool checkGroundCollision() const;

    void setOnGround(bool ground) { onGround = ground; }
    void setVelocityY(float velY) { velocityY = velY; }
    float getVelocityY() const { return velocityY; }
    bool getOnGround() const { return onGround; }

    void moveLeft();
    void moveRight();
    void stopMoving();
    void jump();
    void startParry();
    void startDodge();

    void startAttack();
    void stopAttack();
    std::unique_ptr<Projectile> tryFire();

    bool getIsParrying() const { return isParrying; }
    bool getIsDodging() const { return isDodging; }
    bool isAlive() const { return health > 0; }
    bool getIsAttacking() const { return isAttacking; }
    int getLastDirection() const { return lastDirection; }
};