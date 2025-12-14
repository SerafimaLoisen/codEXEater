#pragma once
#include "GameObject.h"

class Platform : public GameObject {
public:
    Platform(int x, int y, int width);
    void update() override {}
    void render() override;

    int getEndX() const { return x + width; }
    int getEndY() const { return y + height; }

    void renderAt(int screenX, int screenY) const override;
};