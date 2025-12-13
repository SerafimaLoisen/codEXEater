#pragma once
#include "Player.h"
#include <string>

class UIManager {
private:
    static int screenHeight;
    static int screenWidth;
    static bool initialized;

public:
    static void initialize();

    static void renderGameUI(const Player& player, int score);
    static void renderTextAt(int x, int y, const std::string& text);
    static void renderHints();
    static void renderControls();
    static void renderGameOver();

    static bool isInitialized() { return initialized; }
};