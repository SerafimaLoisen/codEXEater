#include "BossRootAttack.h"
#include "BossManager.h"
#include "ConfigManager.h"
#include <cstdlib>

BossRootAttack::BossRootAttack(int cooldown, int warningDuration, int maxLength, int damage, int color)
    : BossAttack(cooldown),
    warningDuration(warningDuration),
    maxLength(maxLength),
    damage(damage),
    color(color) {
}

void BossRootAttack::execute(BossManager& manager, Boss&) {
    auto positions = ConfigManager::getInstance().getBossRootPositions();
    if (positions.empty()) return;

    const auto& p = positions[rand() % positions.size()];

    RootSpawnInstance inst;
    inst.x = p.x;
    inst.y = p.y;
    inst.dir = p.dir;
    inst.warningTimer = warningDuration;
    inst.active = false;

    instances.push_back(inst);
}

void BossRootAttack::updateInstances(BossManager& manager) {
    for (auto& inst : instances) {
        if (inst.active) continue;

        if (inst.warningTimer > 0) {
            inst.warningTimer--;
        }
        else {
            manager.spawnRoot(inst.x, inst.y, inst.dir, maxLength, damage, color);
            inst.active = true;
        }
    }
}

void BossRootAttack::renderWarnings(BossManager& manager) {
    for (auto& inst : instances) {
        if (!inst.active && inst.warningTimer > 0) {
            manager.renderWarning(inst.x, inst.y, color);
        }
    }
}
