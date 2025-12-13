#pragma once

class Boss;
class GameEngine;

class BossAttack {
protected:
    Boss* boss;
    int cooldown;
    int currentCooldown;

public:
    BossAttack(Boss* boss, int cooldown);
    virtual ~BossAttack() = default;

    virtual void update() {
        if (currentCooldown > 0) currentCooldown--;
    }
    virtual void update(GameEngine& engine) { update(); }  // Перегруженная версия

    virtual void execute(GameEngine& engine) = 0;
    virtual bool canExecute() const { return currentCooldown <= 0; }
    virtual void resetCooldown() { currentCooldown = cooldown; }
};