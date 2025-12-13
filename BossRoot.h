#pragma once
#include "Entity.h"

class BossRoot : public Entity {
private:
    int growTimer;
    int maxHeight;
    bool growingUp;  // true = растет вверх, false = растет вниз

public:
    BossRoot(int x, int y, bool growUp, int maxHeight, int damage, int color);

    void update() override;
    void render() override;
    bool isFullyGrown() const;
    int getDamage() const { return getHealth(); }
};