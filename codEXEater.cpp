#include <iostream>
#include <conio.h>
#include <windows.h>
#include "GameEngine.h"
#include "ConfigManager.h"
#include "GraphicsManager.h"
#include <unordered_map>
#include <string>

// Функция для установки размера консоли
void setConsoleSize(int width, int height) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // Устанавливаем размер буфера экрана
    COORD bufferSize;
    bufferSize.X = width;
    bufferSize.Y = height + 10; // Немного больше для UI
    SetConsoleScreenBufferSize(hConsole, bufferSize);

    // Устанавливаем размер окна консоли
    SMALL_RECT windowSize;
    windowSize.Left = 0;
    windowSize.Top = 0;
    windowSize.Right = width - 1;
    windowSize.Bottom = height - 1;
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

    // Скрываем полосы прокрутки
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    csbi.dwSize.X = width;
    csbi.dwSize.Y = height + 10;
    SetConsoleScreenBufferSize(hConsole, csbi.dwSize);

    // Устанавливаем размер окна
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
}

// Функция для скрытия полос прокрутки
void hideScrollBars() {
    HWND consoleWindow = GetConsoleWindow();
    ShowScrollBar(consoleWindow, SB_BOTH, FALSE);
}

// Функция для установки шрифта консоли (опционально)
void setConsoleFont(int fontSize) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX fontInfo;
    fontInfo.cbSize = sizeof(fontInfo);
    GetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);

    // Используем Consolas или другой моноширинный шрифт
    wcscpy_s(fontInfo.FaceName, L"Consolas");
    fontInfo.dwFontSize.X = 0; // Ширина (авто)
    fontInfo.dwFontSize.Y = fontSize; // Высота шрифта

    SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);
}

std::string getLevelNameByChoice(int choice) {
    switch (choice) {
    case 1: return "Level1";
    case 2: return "level2";
    case 3: return "level3";
    case 4: return "level4";
    case 5: return "level5";
    case 6: return "level6";
    case 7: return "level7";
    default: return "";
    }
}

void showMainMenu() {
    system("cls");
    std::cout << "=== BULLET PARRY GAME ===\n\n";
    std::cout << "Available Levels:\n";
    std::cout << "1. Tutorial Level\n";
    std::cout << "2. Level 1\n";
    std::cout << "3. Level 2\n";
    std::cout << "4. Level 3\n";
    std::cout << "5. Level 4\n";
    std::cout << "6. Level 5\n";
    std::cout << "7. Boss Arena\n";
    std::cout << "8. Exit\n\n";
    std::cout << "Select level (1-8): ";
}

void handleInput(GameEngine& game) {
    static bool key1 = false, key2 = false, key3 = false, key4 = false,
        key5 = false, key6 = false, key7 = false;

    // Проверяем состояние всех клавиш движения КАЖДЫЙ КАДР
    bool currentA = (GetAsyncKeyState('A') & 0x8000);
    bool currentD = (GetAsyncKeyState('D') & 0x8000);
    bool currentW = (GetAsyncKeyState('W') & 0x8000);
    bool currentShift = (GetAsyncKeyState(VK_SHIFT) & 0x8000);

    // Обработка движения - проверяем текущее состояние, а не изменение
    if (currentA && !currentD) {
        game.getPlayer().moveLeft();
    }
    else if (currentD && !currentA) {
        game.getPlayer().moveRight();
    }
    else {
        game.getPlayer().stopMoving();
    }

    // Прыжок - при удержании W (автопрыжок)
    if (currentW) {
        game.getPlayer().jump();
    }

    // Атака
    static bool prevShift = false;
    if (currentShift && !prevShift) {
        game.getPlayer().startAttack();
    }
    else if (!currentShift && prevShift) {
        game.getPlayer().stopAttack();
    }
    prevShift = currentShift;

    // Переключение уровней
    bool current1 = (GetAsyncKeyState('1') & 0x8000);
    bool current2 = (GetAsyncKeyState('2') & 0x8000);
    bool current3 = (GetAsyncKeyState('3') & 0x8000);
    bool current4 = (GetAsyncKeyState('4') & 0x8000);
    bool current5 = (GetAsyncKeyState('5') & 0x8000);
    bool current6 = (GetAsyncKeyState('6') & 0x8000);
    bool current7 = (GetAsyncKeyState('7') & 0x8000);

    if (current1 && !key1) game.switchLevel("level1");
    if (current2 && !key2) game.switchLevel("level2");
    if (current3 && !key3) game.switchLevel("level3");
    if (current4 && !key4) game.switchLevel("level4");
    if (current5 && !key5) game.switchLevel("level5");
    if (current6 && !key6) game.switchLevel("level6");
    if (current7 && !key7) game.switchLevel("level7");

    key1 = current1; key2 = current2; key3 = current3; key4 = current4;
    key5 = current5; key6 = current6; key7 = current7;

    // Другие клавиши
    if (_kbhit()) {
        int key = _getch();
        switch (key) {
        case ' ': game.getPlayer().startParry(); break;
        case 's': case 'S': game.getPlayer().startDodge(); break;
        case 'q': case 'Q': game.setGameRunning(false); break;
            // Цифры для быстрого переключения
        case '1': game.switchLevel("level1"); break;
        case '2': game.switchLevel("level2"); break;
        case '3': game.switchLevel("level3"); break;
        case '4': game.switchLevel("level4"); break;
        case '5': game.switchLevel("level5"); break;
        case '6': game.switchLevel("level6"); break;
        case '7': game.switchLevel("level7"); break;
        }
    }
}

void showGameOverMenu(GameEngine& game) {
    // Восстанавливаем нормальный размер консоли для меню
    setConsoleSize(80, 25);

    std::cout << "\n\nGame Over! Your score: " << game.getScore() << "\n";
    std::cout << "Press R to restart current level\n";
    std::cout << "Press M to return to main menu\n";
    std::cout << "Press Q to quit\n";

    while (true) {
        if (_kbhit()) {
            int key = _getch();
            switch (key) {
            case 'r': case 'R': return; // Перезапуск текущего уровня
            case 'm': case 'M':
                game.setGameRunning(false);
                return;
            case 'q': case 'Q':
                game.setGameRunning(false);
                exit(0);
            }
        }
        Sleep(100);
    }
}

int main() {
    SetConsoleOutputCP(65001);

    // Получаем настройки из конфига
    ConfigManager::initialize();
    auto& config = ConfigManager::getInstance();
    int viewportWidth = config.getViewportWidth();
    int viewportHeight = config.getViewportHeight();

    // Устанавливаем размер консоли равным viewport
    setConsoleSize(viewportWidth, viewportHeight + 5); // +5 для UI сверху
    hideScrollBars();
    setConsoleFont(16); // Можно настроить размер шрифта

    // Скрыть курсор
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursor = { 1, 0 };
    SetConsoleCursorInfo(console, &cursor);

    while (true) {
        // Для главного меню восстанавливаем нормальный размер
        setConsoleSize(80, 25);
        showMainMenu();

        int choice = _getch() - '0';

        if (choice == 8) return 0; // Выход

        std::string levelName = getLevelNameByChoice(choice);
        if (levelName.empty()) {
            std::cout << "\nInvalid choice!\n";
            Sleep(1000);
            continue;
        }

        // Устанавливаем игровой размер консоли
        setConsoleSize(viewportWidth, viewportHeight + 5);
        hideScrollBars();

        // Создаем и инициализируем игру с выбранным уровнем
        GameEngine game;
        game.initialize(levelName);

        // Игровой цикл
        while (game.isRunning()) {
            // Обработка ввода
            handleInput(game);

            // Обновление и отрисовка
            game.update();
            game.render();
            Sleep(50);
        }

        // После завершения игры
        showGameOverMenu(game);
    }

    return 0;
}