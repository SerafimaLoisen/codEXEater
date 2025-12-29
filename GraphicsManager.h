#pragma once
#include <string>
#include <vector>
#include <map>

typedef std::map<std::string, std::vector<std::string>> GraphicsMap;

class ComponentsBasedEntity;

class GraphicsManager {
private:
    static GraphicsMap graphics;

public:
    static void loadGraphics(const std::string& filename, const std::string& name);
    static const std::vector<std::string>& getGraphic(const std::string& name);
    static void renderAt(int x, int y, const std::vector<std::string>& graphic, int color = 7);

    static void renderAtAndFitViewport(ComponentsBasedEntity* entityToRender, int cameraX, int cameraY, int viewportWidth, int viewportHeight);
};
