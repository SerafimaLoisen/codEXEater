#pragma once

#include "Entity.h"

class Boss : public Entity {
public:
    Boss(
        int x,
        int y,
        int w,
        int h,
        int hp,
        int phase2HP,
        int phase3HP
    );

    void update();
    void render();
    void renderAt(int screenX, int screenY) const override;

    int getPhase() const;

    void restart();

private:
    int phase2HP;
    int phase3HP;
    int currentPhase;

    int maxHP;
};
