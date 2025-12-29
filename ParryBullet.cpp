#include "ParryBullet.h"
#include "ConfigManager.h"
#include "GraphicsManager.h"

ParryBullet::ParryBullet(int x, int y, int direction)
    : Projectile(x, y, 1, 1,
        ConfigManager::getInstance().getParryBulletSpeed(),
        direction, 13) {
}

void ParryBullet::render() {
    GraphicsManager::renderAt(x, y, GraphicsManager::getGraphic("parry_bullet"), 13);
}

void ParryBullet::renderAt(int screenX, int screenY) const {
    GraphicsManager::renderAt(screenX, screenY, GraphicsManager::getGraphic("parry_bullet"), 13);
}

//#include "ParryBullet.h"
//#include "ConfigManager.h"
//#include "GraphicsManager.h"
//#include <cmath>
//
//ParryBullet::ParryBullet(int x, int y, float vx, float vy)
//    : Bullet(x, y, 1)  // direction ��� �� �����
//{
//    enemy = true;
//    damage = ConfigManager::getInstance().getParryBulletDamage();
//    color = ConfigManager::getInstance().getParryBulletColor();
//
//    setVelocity(vx, vy);
//}
//
//void ParryBullet::parry(float newSpeed) {
//    // �������� �� X � ��������
//    float dirX = (velocityX >= 0.f) ? 1.f : -1.f;
//    setVelocity(-dirX * newSpeed, velocityY * 0.5f);
//
//    enemy = false; // ������ ��� "�������������" ����
//}
//
//void ParryBullet::render() {
//    GraphicsManager::renderAt(x, y,
//        GraphicsManager::getGraphic("parry_bullet"),
//        color
//    );
//}