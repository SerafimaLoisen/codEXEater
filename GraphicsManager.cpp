#include "GraphicsManager.h"
#include <fstream>
#include <iostream>
#include <windows.h>
#include "ComponentsBasedEntity.h"

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
        // RESTRICT MAXIMUM WIDTH OF STRING TO DIFFERENCE BETWEEN 
    }
    SetConsoleTextAttribute(hConsole, 7);
}

void GraphicsManager::renderAtAndFitViewport(ComponentsBasedEntity* entityToRender, int cameraX, int cameraY, int viewportWidth, int viewportHeight)
{
    /*
    int spriteWidth = 1;
    int spriteHeight = graphic.size();
    std::vector<std::string> altGraphics = std::vector<std::string>(graphic);

    for (const std::string& line : altGraphic) {
        if (line.length() > spriteWidth)
            spriteWidth = line.length();
    }
    */

    int x = entityToRender->getX();
    int y = entityToRender->getY();

    int spriteWidth = entityToRender->getWidth();
    int spriteHeight = entityToRender->getHeight();

    int color = entityToRender->getColor();

    std::vector<std::string> altGraphics = entityToRender->GetSprite();

    // SOME PART OF SPRITE IS OUT OF VIEWPORT'S LEFT BORDER
    if ((x < cameraX) && ((x + spriteWidth) > cameraX)) 
    {
        for (std::string& line : altGraphics) {
            int diff = x + line.length();
            diff -= cameraX;
            if (diff > 0) {
                line.erase(line.begin(), line.begin() + (line.length() - diff));
                x = cameraX;
            }
        }

    } // SOME PART OF SPRITE IS OUT OF VIEWPORT'S RIGHT BORDER
    else if ((x < (cameraX + viewportWidth)) && (x + spriteWidth > (cameraX + viewportWidth)))
    {
        for (std::string& line : altGraphics) {
            int diff = x + line.length();
            diff -= cameraX + viewportWidth;
            if (diff > 0) {
                line.erase(line.length() - diff);
            }
        }
    }

    // SOME PART OF SPRITE IS OUT OF VIEWPORT'S TOP BORDER
    if ((y < cameraY) && ((y + spriteHeight) > cameraY)) {

        int diff = cameraY-y;
        altGraphics.erase(altGraphics.begin(), altGraphics.begin()+diff);
        y = cameraY;

    } // SOME PART OF SPRITE IS OUT OF VIEWPORT'S BOTTOM BORDER
    else if ((y < (cameraY + viewportHeight)) && ((y + spriteHeight) > (cameraY + viewportHeight)))
    {
        int diff = (y + spriteHeight) - (cameraY + viewportHeight);
        altGraphics.erase(altGraphics.begin() + (spriteHeight - diff), altGraphics.end());
    }

    renderAt(x-cameraX, y-cameraY, altGraphics, color);
}
