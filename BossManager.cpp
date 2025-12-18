#include "BossManager.h"
#include "Boss.h"
#include "BossAttack.h"
#include "BossBulletAttack.h"
#include "BossRootAttack.h"
#include "BossRoot.h"
#include "Player.h"
#include "GameEngine.h"
#include "GraphicsManager.h"
#include "ConfigManager.h"

BossManager::BossManager(GameEngine* engine)
    : engine(engine) {
    auto& cfg = ConfigManager::getInstance();

    boss = std::make_unique<Boss>(
        cfg.getBossStartX(),
        cfg.getBossStartY(),
        cfg.getBossWidth(),
        cfg.getBossHeight(),
        cfg.getBossHealth(),
        cfg.getBossPhase2HP(),
        cfg.getBossPhase3HP()
    );

    bulletAttacks.push_back(
        std::make_unique<BossBulletAttack>(cfg.getFollowBulletCooldown(), &engine->getPlayer())
    );

    rootAttacks.push_back(
        std::make_unique<BossRootAttack>(
            cfg.getBossRootCooldown(),
            cfg.getBossRootWarningDuration(),
            cfg.getBossRootMaxLength(),
            cfg.getBossRootDamage(),
            cfg.getBossRootColor()
        )
    );
}

BossManager::~BossManager() = default;

void BossManager::update() {
    if (!boss) return;
    boss->update();
    int phase = boss->getPhase();

    if (phase == 1 || phase == 3)
        for (auto& attack : bulletAttacks)
            attack->update(*this, *boss);

    if (phase == 2 || phase == 3)
        for (auto& attack : rootAttacks) {
            attack->update(*this, *boss);
            if (auto r = dynamic_cast<BossRootAttack*>(attack.get()))
                r->updateInstances(*this);
        }

    for (int i = (int)roots.size() - 1; i >= 0; --i) {
        roots[i]->update();
        if (roots[i]->getHealth() <= 0)
            roots.erase(roots.begin() + i);
    }
}

void BossManager::render() {
    if (!boss) return;
    boss->render();
    for (auto& root : roots) root->render();
    for (auto& attack : rootAttacks)
        if (auto r = dynamic_cast<BossRootAttack*>(attack.get()))
            r->renderWarnings(*this);
}

void BossManager::renderWarning(int x, int y, int color) {
    GraphicsManager::renderAt(x, y, GraphicsManager::getGraphic("warning"), color);
}

void BossManager::spawnRoot(int x, int y, GrowDirection dir, int maxLength, int damage, int color) {
    auto& cfg = ConfigManager::getInstance();
    auto root = std::make_unique<BossRoot>(
        x, y, dir, maxLength, damage, color, cfg.getBossRootCollisionCooldown()
    );
    roots.push_back(std::move(root));
}

void BossManager::spawnBossBullet(std::unique_ptr<Projectile> bullet) {
    if (engine && bullet) engine->addEnemyBullet(std::move(bullet));
}

void BossManager::checkPlayerCollisions(Player& player) {
    for (auto& root : roots) {
        if (checkAABB(player.getX(), player.getY(), player.getWidth(), player.getHeight(),
            root->getX(), root->getY(), root->getWidth(), root->getHeight())) {
            if (root->canCollide()) {
                player.takeDamage(root->getDamage());
                root->takeDamage(1);
                root->resetCollisionCooldown();
            }
        }
    }
}

void BossManager::checkPlayerBulletCollisions(std::vector<std::unique_ptr<Projectile>>& bullets) {
    for (int i = (int)roots.size() - 1; i >= 0; --i) {
        auto& root = roots[i];
        for (int j = (int)bullets.size() - 1; j >= 0; --j) {
            auto& bullet = bullets[j];
            if (bullet->isEnemy()) continue;

            if (checkAABB(bullet->getX(), bullet->getY(), bullet->getWidth(), bullet->getHeight(),
                root->getX(), root->getY(), root->getWidth(), root->getHeight())) {
                root->takeDamage(bullet->getDamage());
                bullets.erase(bullets.begin() + j);
                if (root->getHealth() <= 0) {
                    roots.erase(roots.begin() + i);
                    break;
                }
            }
        }
    }
}

bool BossManager::checkAABB(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

Boss& BossManager::getBoss() { return *boss; }
GameEngine* BossManager::getEngine() { return engine; }
