#include "UIManager.h"
#include "ConfigManager.h"
#include <iostream>

int UIManager::screenHeight = 0;
int UIManager::screenWidth = 0;
bool UIManager::initialized = false;

void UIManager::initialize() {
    if (initialized) return;

    auto& config = ConfigManager::getInstance();
    screenHeight = config.getScreenHeight();
    screenWidth = config.getScreenWidth();

    initialized = true;
}

void UIManager::renderGameUI(const Player& player, int score) {
    if (!initialized) initialize();

    std::string health = "HEALTH: ";
    for (int i = 0; i < player.getMaxHealth(); i++) {
        health += (i < player.getHealth()) ? "0 " : "  ";
    }
    renderTextAt(2, 2, health);

    std::string scoreText = "SCORE: " + std::to_string(score);
    renderTextAt(2, 3, scoreText);
}

void UIManager::renderTextAt(int x, int y, const std::string& text) {
    std::cout << "\033[" << y << ";" << x << "H" << text;
}

void UIManager::renderHints() {
    if (!initialized) initialize();

    renderTextAt(38, 2, "TUTORIAL");
    renderTextAt(23, 3, "o - simple bullet  * - parry bullet");
}

void UIManager::renderControls() {
    if (!initialized) initialize();

    std::string controls = "Controls: [A][D]-Move  [W]-Jump  [SPACE]-Parry  [S]-Dodge  [Q]-Quit";
    renderTextAt(6, screenHeight - 5, controls);
}

void UIManager::renderGameOver() {
    if (!initialized) initialize();
    std::cout << "\033[2J\033[1;1H";
    std::string gameOverText = "GAME OVER";
    int x = (screenWidth - gameOverText.length()) / 2;
    int y = screenHeight / 2;

    renderTextAt(x, y, gameOverText);
}