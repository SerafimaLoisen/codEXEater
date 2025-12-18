#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>
#include "GrowDirection.h"

struct RootSpawn {
    int x;
    int y;
    GrowDirection dir;
};

class ConfigManager {
private:
    static ConfigManager* instance;
    std::unordered_map<std::string, std::string> config;
    std::unordered_map<std::string, int> intCache;
    std::unordered_map<std::string, float> floatCache;

    void loadConfigInternal();

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
    int getPlayerColor();
    int getPlayerCooldown();
    int getBulletSpeed();
    int getParryBulletSpeed();
    int getParryDuration();
    int getDodgeDuration();
    int getDodgeDistance();
    int getParryRange();
    int getPlayerBulletColor();
    int getBossStartX();
    int getBossStartY();
    int getBossWidth();
    int getBossHeight();
    int getBossHealth();
    int getBossPhase2HP();
    int getBossPhase3HP();
    int getFollowBulletDamage();
    int getFollowBulletSpeed();
    int getFollowBulletColor();
    int getFollowBulletCooldown();
    int getBossRootWarningDuration();
    int getFollowBulletFollowDuration();
    int getBossRootGrowDuration();
    int getBossRootMaxLength();
    int getBossRootDamage();
    int getBossRootCooldown();
    int getBossRootColor();
    int getBossRootHealth(); // Здоровье корня (по умолчанию 10)
    int getBossRootCollisionCooldown();
    std::vector<RootSpawn> getBossRootPositions();
    int getLevelCount();
    int getBulletColor();       // цвет обычной пули
    int getBulletDamage();      // урон обычной пули
    int getParryBulletColor();  // цвет паррируемой пули
    int getParryBulletDamage(); // урон паррируемой пули
    std::string getLevelName(int index);
    std::string getLevelFile(int index);
    std::string getLevelDescription(int index);
};