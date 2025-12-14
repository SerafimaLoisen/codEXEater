//#include "BossRoot.h"
//#include "ConfigManager.h"
//#include "GraphicsManager.h"
//#include <iostream>
//
//BossRoot::BossRoot(int x, int y, bool growUp, int maxHeight, int damage, int color)
//    : Entity(x, y, 1, 1, damage, color),
//    growTimer(maxHeight),
//    maxHeight(maxHeight),
//    growingUp(growUp) {
//    height = 1;
//}
//
//void BossRoot::update() {
//    if (growTimer > 0 && height < maxHeight) {
//        growTimer--;
//        height++;
//
//        if (growingUp) {
//            y--; // Растем вверх
//        }
//    }
//}
//
//void BossRoot::render() {
//    if (!isAlive()) return;
//
//    // Создаем графику корня как вектор строк
//    std::vector<std::string> rootGraphic;
//
//    for (int i = 0; i < height; i++) {
//        // Разные символы для разных частей корня
//        char symbol = '|';
//        if (i == 0) symbol = '#';  // Основание
//        else if (i == height - 1) symbol = '^';  // Верхушка
//
//        // Создаем строку с одним символом
//        rootGraphic.push_back(std::string(1, symbol));
//    }
//
//    // Определяем позицию для отрисовки
//    int renderX = x;
//    int renderY = growingUp ? y : y - height + 1;
//
//    // Используем существующий renderAt
//    GraphicsManager::renderAt(renderX, renderY, rootGraphic, color);
//}
//
//
//bool BossRoot::isFullyGrown() const {
//    return height >= maxHeight || growTimer <= 0;
//}