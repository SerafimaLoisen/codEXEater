#include "BossRoot.h"
#include "GraphicsManager.h"

BossRoot::BossRoot(int x, int y, GrowDirection dir, int maxLength, int dmg, int color, int cooldownMax)
    : startX(x), startY(y), direction(dir),
    length(0), maxLength(maxLength),
    damage(dmg), color(color), health(maxLength),
    collisionCooldownMax(cooldownMax), collisionCooldown(0) {
    isGrown = false;

    //if ((dir == GrowDirection::Left)
    //    || (dir == GrowDirection::Right)) {
    //    graphic = GraphicsManager::getGraphic("root_h");
    //}
    //else {
    //    graphic = GraphicsManager::getGraphic("root");
    //}

    graphic = GraphicsManager::getGraphic("root");
}

void BossRoot::update() {
    if ((length < maxLength)
        && (!isGrown)) {
        length++;
        if (length >= maxLength) {
            isGrown = true;
        }
    }

    if (collisionCooldown > 0) {
        collisionCooldown--;
    }
}

void BossRoot::render() {
    //for (int i = 0; i < length; ++i) {
    //    int drawX = startX;
    //    int drawY = startY;
    //    switch (direction) {
    //    case GrowDirection::Up:    drawY = (startY+length) - i; break;
    //    case GrowDirection::Down:  drawY = (startY+length) + i; break;
    //    case GrowDirection::Left:  drawX = (startX+length) - i; break;
    //    case GrowDirection::Right: drawX = (startX+length) + i; break;
    //    }
    //    GraphicsManager::renderAt(drawX, drawY,
    //        GraphicsManager::getGraphic("root"),
    //        color);
    //}

    int drawX = startX;
    int drawY = startY;

    switch (direction) {
    case GrowDirection::Up:    drawY = (startY - length); break;
    //case GrowDirection::Down:  drawY = (startY + length); break;
    //case GrowDirection::Left:  drawX = (startX - length); break;
    //case GrowDirection::Right: drawX = (startX + length); break;
    }

    GraphicsManager::renderAt(drawX, drawY, graphic, color);
}

bool BossRoot::canCollide() {
    if (collisionCooldown == 0) {
        collisionCooldown = collisionCooldownMax;
        return true;
    }
    return false;
}

void BossRoot::resetCollisionCooldown() {
    collisionCooldown = collisionCooldownMax;
}

// ===== хитбокс, учитывающий рост =====
int BossRoot::getX() const {
    //if (direction == GrowDirection::Left)
    //    return startX - length + 1;
    return startX;
}

int BossRoot::getY() const {
    if (direction == GrowDirection::Up)
        return startY - length + 1;
    return startY;
}

int BossRoot::getWidth() const {
    //if (direction == GrowDirection::Left || direction == GrowDirection::Right)
    //    return length;
    return 1;
}

int BossRoot::getHeight() const {
    //if (direction == GrowDirection::Up || direction == GrowDirection::Down)
    //    return length;
    if (direction == GrowDirection::Up)
        return length;
    return 1;
}
