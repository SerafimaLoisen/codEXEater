// SidePlatform.h
#pragma once
#include "GameObject.h"
#include <string>

class SidePlatform : public GameObject {
public:
    SidePlatform(int x, int y, int height);

    // Реализуем чисто виртуальные методы
    void update() override;
    void render() override;
    void renderAt(int screenX, int screenY) const override;
};