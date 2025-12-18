#pragma once
#include "GameObject.h"
#include <vector>

class Projectile : public GameObject {
// !!! From general
protected:
    int speed;
    int direction; // 1 = Ð²Ð¿Ñ€Ð°Ð²Ð¾, -1 = Ð²Ð»ÐµÐ²Ð¾

    std::vector<float> initCoord = { 0, 0 };
    std::vector<float> coord = { 0, 0 };
    std::vector<float> velocity = { 0, 0 };
    bool useFloatCoord = false;
    float maxTravelDistance = 1000;

public:
    Projectile(int x, int y, int width, int height, int speed, int direction, int color, 
        bool _useFloatCoord = false, std::vector<float> _coord = { 0, 0 }, 
        std::vector<float> _velocity = { 0,0 });

    void update() override;
    virtual void renderAt(int screenX, int screenY) const;

    void setSpeed(int newSpeed) { speed = newSpeed; }
    void setVelocity(std::vector<float> newVelocity) { velocity = newVelocity; }
    void setUseFloatCoord(bool _useFloatCoord) { useFloatCoord = _useFloatCoord; }
    void setMaxTravelDistance(float _maxTravelDistance) { maxTravelDistance = _maxTravelDistance; }
    void setDirection(int newDirection) { direction = newDirection; }
    int getDirection() const { return direction; }
    int getSpeed() const { return speed; }
	
// !!! From sima
//protected:
//    // --- ÏËÀÂÍÀß ïîçèöèÿ ---
//    float posX;
//    float posY;
//
//    // --- ñêîðîñòü ---
//    float velocityX = 0.f;
//    float velocityY = 0.f;
//
//    bool enemy;
//    int damage;
//
//public:
//    Projectile(
//        int x,
//        int y,
//        int width,
//        int height,
//        int color,
//        bool enemy,
//        int damage
//    );
//
//    virtual ~Projectile() = default;
//
//    virtual void update();

    bool isEnemy() const { return enemy; }
    int getDamage() const { return damage; }

    void setVelocity(float vx, float vy);
};
