//#include "BossBulletAttack.h"
//#include "Boss.h"
//#include "GameEngine.h"
//#include "Bullet.h"
//#include <cstdlib>
//
//BossBulletAttack::BossBulletAttack(Boss* boss, int cooldown, int speed, int color)
//    : BossAttack(boss, cooldown), bulletSpeed(speed), bulletColor(color) {
//    currentCooldown = 0;  // Начинаем сразу
//}
//
//void BossBulletAttack::update() {
//    decreaseCooldown();
//}
//
//void BossBulletAttack::execute(GameEngine& engine) {
//    // Стреляем в сторону игрока
//    int bossX = boss->getX();
//    int bossY = boss->getY();
//    int bossHeight = boss->getHeight();
//
//    // Случайная высота для выстрела
//    int bulletY = bossY + (std::rand() % bossHeight);
//
//    // Создаем пулю (летит влево, к игроку)
//    auto bullet = std::make_unique<Bullet>(bossX - 1, bulletY, -1);
//    bullet->setSpeed(bulletSpeed);
//    bullet->setColor(bulletColor);
//
//    // Добавляем в движок
//    // Нужно добавить метод в GameEngine для добавления вражеских пуль
//    // или использовать существующий projectiles
//}