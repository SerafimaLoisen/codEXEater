#include "UIManager.h"
#include "ConfigManager.h"
#include <iostream>
#include "Boss.h"

// Инициализация статических членов
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

void UIManager::renderBossHealth(const Boss& boss) {
    if (!initialized) initialize();

    constexpr int BAR_WIDTH = 20;

    int hp = boss.getHealth();
    int maxHp = boss.getMaxHealth();

    int filled = (hp * BAR_WIDTH) / maxHp;

    std::string bar = "BOSS: [";
    for (int i = 0; i < BAR_WIDTH; ++i) {
        bar += (i < filled) ? "0" : " ";
    }
    bar += "]";

    int x = (screenWidth - bar.length()) / 2;
    int y = 1;

    renderTextAt(x, y, bar);
}

void UIManager::renderBossPhase(int phase) {
    if (!initialized) initialize();

    std::string phaseText = "BOSS PHASE: ";

    switch (phase) {
    case 1: phaseText += "I"; break;
    case 2: phaseText += "II"; break;
    case 3: phaseText += "III"; break;
    default: phaseText += "?"; break;
    }

    // позиция — справа сверху
    renderTextAt(screenWidth - 20, 2, phaseText);
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

void UIManager::renderVictory() {
    if (!initialized) initialize();
    std::cout << "\033[2J\033[1;1H";
    std::string gameOverText = "VICTORY!";
    int x = (screenWidth - gameOverText.length()) / 2;
    int y = screenHeight / 2;

    renderTextAt(x, y, gameOverText);
}