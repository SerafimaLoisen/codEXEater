#pragma once
#include "Player.h"
#include <string>

class Boss;

class UIManager {
private:
    static int screenHeight;
    static int screenWidth;
    static bool initialized;

public:
    static void initialize();

    static void renderGameUI(const Player& player, int score);
    static void renderTextAt(int x, int y, const std::string& text);
    static void renderBossHealth(const Boss& boss);
    static void renderBossPhase(int phase);
    static void renderHints();
    static void renderControls();
    static void renderGameOver();
    static void renderVictory();

    static bool isInitialized() { return initialized; }
};