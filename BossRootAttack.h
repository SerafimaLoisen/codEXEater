#pragma once
#include "BossAttack.h"
#include "GrowDirection.h"
#include <vector>

class BossManager;
class Boss;

struct RootSpawnInstance {
    int x, y;
    GrowDirection dir;
    int warningTimer;
    bool active;
};

class BossRootAttack : public BossAttack {
public:
    BossRootAttack(int cooldown, int warningDuration, int maxLength, int damage, int color);

    void execute(BossManager& manager, Boss& boss) override;
    void updateInstances(BossManager& manager);
    void renderWarnings(BossManager& manager);

private:
    int warningDuration;
    int maxLength;
    int damage;
    int color;

    std::vector<RootSpawnInstance> instances;
};
