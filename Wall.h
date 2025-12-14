// Wall.h
#pragma once
#include "GameObject.h"
#include <vector>
#include <string>

class Wall : public GameObject {
public:
    Wall(int x, int y, int width, int height);

    void update() override {}  // Пустая реализация

    void render() override;    // Без const

    void renderAt(int screenX, int screenY) const override;
};