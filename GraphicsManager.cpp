#include "GraphicsManager.h"
#include <fstream>
#include <iostream>
#include <windows.h>

GraphicsMap GraphicsManager::graphics;

void GraphicsManager::loadGraphics(const std::string& filename, const std::string& name) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Can't open graphics file: " << filename << std::endl;
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();

    graphics[name] = lines;
}

const std::vector<std::string>& GraphicsManager::getGraphic(const std::string& name) {
    return graphics[name];
}

void GraphicsManager::renderAt(int x, int y, const std::vector<std::string>& graphic, int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord;
    SetConsoleTextAttribute(hConsole, color);

    for (size_t i = 0; i < graphic.size(); i++) {
        coord.X = x;
        coord.Y = y + i;
        SetConsoleCursorPosition(hConsole, coord);
        std::cout << graphic[i];
    }
    SetConsoleTextAttribute(hConsole, 7);
}
