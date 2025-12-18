#pragma once

#include "BossAttack.h"

class Player;

class BossBulletAttack : public BossAttack {
public:
    BossBulletAttack(int cooldown, Player* player);

protected:
    void execute(BossManager& manager, Boss& boss) override;

private:
    Player* player;
};
