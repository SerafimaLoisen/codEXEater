#pragma once
#include <string>
#include <unordered_map>

class ConfigManager {
private:
    static ConfigManager* instance;
    std::unordered_map<std::string, std::string> config;
    std::unordered_map<std::string, int> intCache;
    std::unordered_map<std::string, float> floatCache;

    void loadConfigInternal();
    char getChar(const std::string& key, char defaultValue = '+');

public:
    static ConfigManager& getInstance();
    static void initialize();

    int getInt(const std::string& key);
    float getFloat(const std::string& key);
    std::string getString(const std::string& key);
    static void reload();

    // Кэшированные геттеры
    int getScreenWidth();
    int getScreenHeight();
    int getPlayerStartX();
    int getPlayerStartY();
    int getPlayerWidth();
    int getPlayerHeight();
    int getPlayerHealth();
    int getBulletSpeed();
    int getParryBulletSpeed();
    int getParryDuration();
    int getDodgeDuration();
    int getDodgeDistance();
    int getParryRange();
    int getPlayerBulletSpeed();
    int getPlayerFireRate();
    int getPlayerBulletColor();
    int getBossStartX();
    int getBossStartY();
    int getBossWidth();
    int getBossHeight();
    int getBossHealth();
    int getBossPhase2HP();
    int getBossPhase3HP();
    int getBossBulletSpeed();
    int getBossBulletColor();
    int getBossBulletCooldown();
    int getBossRootWarningDuration();
    int getBossRootGrowDuration();
    int getBossRootDamage();
    int getBossRootCooldown();
    int getBossRootColor();

    int getViewportWidth();
    int getViewportHeight();
    int getCameraFollowSpeed();

    int getWorldWidth(const std::string& levelName = "tutorial");
    int getWorldHeight(const std::string& levelName = "tutorial");

    int getLevelCount();
    std::string getLevelName(int index);
    std::string getLevelFile(int index);
    std::string getLevelDescription(int index);

    char getCheckpointSymbol();          // Символ чекпоинта в файле уровня
    int getCheckpointActivationRange();  // Радиус активации чекпоинта
    int getCheckpointActiveColor();      // Цвет активного чекпоинта
    int getCheckpointInactiveColor();    // Цвет неактивного чекпоинта
};