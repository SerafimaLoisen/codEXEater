#pragma once

class Camera {
private:
    int viewportWidth;   // Ширина видимой области
    int viewportHeight;  // Высота видимой области
    int worldWidth;      // Ширина всего уровня
    int worldHeight;     // Высота всего уровня
    int x;               // Позиция камеры по X
    int y;               // Позиция камеры по Y

public:
    Camera(int viewportW, int viewportH, int worldW, int worldH);

    void setPosition(int newX, int newY);
    void centerOn(int targetX, int targetY);

    int worldToScreenX(int worldX) const;
    int worldToScreenY(int worldY) const;

    bool isInViewport(int worldX, int worldY, int width, int height) const;


    int ourClamp(int value, int min, int max);

    // Геттеры
    int getX() const { return x; }
    int getY() const { return y; }
    int getViewportWidth() const { return viewportWidth; }
    int getViewportHeight() const { return viewportHeight; }

    void updateViewportSize(int newWidth, int newHeight);
};