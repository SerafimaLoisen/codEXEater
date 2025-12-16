#pragma once
#include "GameObject.h"
#include <vector>

class Projectile : public GameObject {
protected:
    int speed;
    int direction; // 1 = вправо, -1 = влево

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

    virtual void render() override = 0;
};