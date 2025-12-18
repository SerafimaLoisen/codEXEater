#pragma once

class Boss;
class BossManager;

class BossAttack {
protected:
    int cooldown;
    int timer;

public:
    explicit BossAttack(int cooldown)
        : cooldown(cooldown), timer(0) {
    }

    virtual ~BossAttack() = default;

    void update(BossManager& manager, Boss& boss) {
        if (timer > 0) {
            --timer;
            return;
        }

        execute(manager, boss);
        timer = cooldown;
    }

protected:
    virtual void execute(BossManager& manager, Boss& boss) = 0;
};
