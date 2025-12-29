#include "Boss.h"
#include "GraphicsManager.h"

Boss::Boss(
    int x,
    int y,
    int w,
    int h,
    int hp,
    int phase2HP,
    int phase3HP
)
    : Entity(x, y, w, h, hp, 12),
    phase2HP(phase2HP),
    phase3HP(phase3HP),
    currentPhase(1) {
    //health = 1;
    maxHP = hp;
}

void Boss::update() {
    if (health <= phase3HP) currentPhase = 3;
    else if (health <= phase2HP) currentPhase = 2;
    else currentPhase = 1;
}

void Boss::render() {
    GraphicsManager::renderAt(x, y, GraphicsManager::getGraphic("boss"), 12);
}

void Boss::renderAt(int screenX, int screenY) const
{
    GraphicsManager::renderAt(screenX, screenY, GraphicsManager::getGraphic("boss"), 12);
}

int Boss::getPhase() const {
    return currentPhase;
}

void Boss::restart()
{
    currentPhase = 1;
    health = maxHP;
}
