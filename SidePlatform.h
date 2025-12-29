#pragma once
#include "GameObject.h"
#include <string>

class SidePlatform : public GameObject {
public:
    SidePlatform(int x, int y, int height);

    void update() override;
    void render() override;
    void renderAt(int screenX, int screenY) const override;
};