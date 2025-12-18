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

    int getPhase() const;

private:
    int phase2HP;
    int phase3HP;
    int currentPhase;
};
