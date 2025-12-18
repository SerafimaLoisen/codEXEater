#include <iostream>
#include <conio.h>
#include <windows.h>
#include "GameEngine.h"
#include "Player.h"
#include "ConfigManager.h"
#include "GraphicsManager.h"
#include <unordered_map>
#include <string>

int main() {
    SetConsoleOutputCP(65001);

    // Скрыть курсор
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursor = { 1, 0 };
    SetConsoleCursorInfo(console, &cursor);

    while (true) {
        system("cls");
        std::cout << "=== BULLET PARRY GAME ===\n\n";
        std::cout << "1. Tutorial Level\n";
        std::cout << "2. Boss Arena\n";
        std::cout << "3. Exit\n\n";
        std::cout << "Select level (1-3): ";

        int choice = _getch() - '0';

        GameEngine game;

        switch (choice) {
        case 1: // Tutorial
            game.initialize("tutorial");
            break;

        case 2: // Boss
            game.initialize("boss");
            break;

        case 3: // Exit
            return 0;

        default:
            std::cout << "\nInvalid choice!\n";
            Sleep(1000);
            continue;
        }

        game.render();

        // ОБЩИЙ ИГРОВОЙ ЦИКЛ ДЛЯ ЛЮБОГО УРОВНЯ
        while (game.isRunning()) {
            static bool keyA = false, keyD = false;

            bool currentA = (GetAsyncKeyState('A') & 0x8000);
            bool currentD = (GetAsyncKeyState('D') & 0x8000);

            if (currentA && !keyA) {
                game.getPlayer().moveLeft();
            }
            else if (!currentA && keyA) {
                game.getPlayer().stopMoving();
            }

            if (currentD && !keyD) {
                game.getPlayer().moveRight();
            }
            else if (!currentD && keyD) {
                game.getPlayer().stopMoving();
            }
            static bool shiftWasPressed = false;
            bool shiftPressed = (GetAsyncKeyState(VK_SHIFT) & 0x8000);

            // НАЧАЛО АТАКИ при нажатии Shift
            if (shiftPressed && !shiftWasPressed) {
                game.getPlayer().startAttack();
            }
            // ОКОНЧАНИЕ АТАКИ при отпускании Shift
            else if (!shiftPressed && shiftWasPressed) {
                game.getPlayer().stopAttack();
            }

            shiftWasPressed = shiftPressed;

            keyA = currentA;
            keyD = currentD;

            if (_kbhit()) {
                int key = _getch();

                switch (key) {
                case 'a': case 'A': game.getPlayer().moveLeft(); break;
                case 'd': case 'D': game.getPlayer().moveRight(); break;
                case 'w': case 'W': game.getPlayer().jump(); break;
                case ' ': game.getPlayer().startParry(); break;
                case 's': case 'S': game.getPlayer().startDodge(); break;
                case 'q': case 'Q': return 0;
                }

                if (GetAsyncKeyState('A') & 0x8000) {
                    game.getPlayer().moveLeft();
                }
                else if (GetAsyncKeyState('D') & 0x8000) {
                    game.getPlayer().moveRight();
                }
                else {
                    game.getPlayer().stopMoving();
                }
            }

            game.update();
            game.render();
            Sleep(20);
        }

        // После завершения игры спрашиваем, что дальше
        std::cout << "\nPress R to restart, M for menu, Q to quit: ";
        int key = _getch();
        if (key == 'q' || key == 'Q') break;
        if (key == 'm' || key == 'M') continue;
        // R - цикл продолжается с тем же уровнем
    }

    return 0;
}