#include "BossAttack.h"
#include "Boss.h"

BossAttack::BossAttack(Boss* boss, int cooldown)
    : boss(boss), cooldown(cooldown), currentCooldown(0) {
}
/*
void BossAttack::update() {
    // Базовый update - уменьшаем кулдаун
    if (currentCooldown > 0) {
        currentCooldown--;
    }
}
*/