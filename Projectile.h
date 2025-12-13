#pragma once
#include "GameObject.h"

class Projectile : public GameObject {
protected:
    int speed;
    int direction; // 1 = вправо, -1 = влево

public:
    Projectile(int x, int y, int width, int height, int speed, int direction, int color);

    void update() override;

    void setSpeed(int newSpeed) { speed = newSpeed; }
    void setDirection(int newDirection) { direction = newDirection; }
    int getDirection() const { return direction; }
    int getSpeed() const { return speed; }

    virtual void render() override = 0;
};