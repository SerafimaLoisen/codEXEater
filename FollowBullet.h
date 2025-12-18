#pragma once
#include "Bullet.h"

class Player;

class FollowBullet : public Bullet {
    Player* target;
    int homingTimer;

public:
    FollowBullet(int x, int y, Player* target, int homingDuration);
    void update() override;
};
