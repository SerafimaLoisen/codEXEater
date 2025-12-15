#pragma once
#include "Entity.h"
#include "Bullet.h"
#include <memory> 

class Platform;
class SidePlatform;

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
    int playerBulletSpeed;
    int playerFireRate;
    int playerBulletColor;

    int worldWidth;
    int worldHeight;

public:
    Player(int x, int y, int worldWidth, int worldHeight);

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
    std::shared_ptr<Projectile> tryFire();

    bool getIsParrying() const { return isParrying; }
    bool getIsDodging() const { return isDodging; }
    bool isAlive() const { return health > 0; }
    bool getIsAttacking() const { return isAttacking; }
    int getLastDirection() const { return lastDirection; }
    void renderAt(int screenX, int screenY) const override;

    bool isCollidingWithSidePlatform(const SidePlatform& platform, bool& fromLeft);

    void setVelocityX(float velX) { velocityX = velX; }
    float getVelocityX() const { return velocityX; }
};