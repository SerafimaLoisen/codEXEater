#include "ConfigManager.h"
#include <fstream>
#include <iostream>
#include <algorithm>

ConfigManager* ConfigManager::instance = nullptr;

ConfigManager& ConfigManager::getInstance() {
    if (!instance) {
        instance = new ConfigManager();
    }
    return *instance;
}

void ConfigManager::initialize() {
    getInstance().loadConfigInternal();
}

void ConfigManager::loadConfigInternal() {
    std::ifstream file("config.txt");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open config.txt" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == '#') continue;

        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Убираем пробелы вокруг ключа и значения
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            config[key] = value;
        }
    }
}

int ConfigManager::getInt(const std::string& key) {
    // Используем кэш для избежания повторного парсинга
    if (intCache.find(key) != intCache.end()) {
        return intCache[key];
    }

    if (config.find(key) == config.end()) {
        std::cerr << "Warning: Config key '" << key << "' not found, using default 0" << std::endl;
        return 0;
    }

    try {
        int value = std::stoi(config[key]);
        intCache[key] = value; // Сохраняем в кэш
        return value;
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing int for key '" << key << "': " << e.what() << std::endl;
        return 0;
    }
}

float ConfigManager::getFloat(const std::string& key) {
    if (floatCache.find(key) != floatCache.end()) {
        return floatCache[key];
    }

    if (config.find(key) == config.end()) {
        std::cerr << "Warning: Config key '" << key << "' not found, using default 0.0" << std::endl;
        return 0.0f;
    }

    try {
        float value = std::stof(config[key]);
        floatCache[key] = value;
        return value;
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing float for key '" << key << "': " << e.what() << std::endl;
        return 0.0f;
    }
}

std::string ConfigManager::getString(const std::string& key) {
    if (config.find(key) == config.end()) {
        std::cerr << "Warning: Config key '" << key << "' not found, using empty string" << std::endl;
        return "";
    }
    return config[key];
}

char ConfigManager::getChar(const std::string& key, char defaultValue) {
    std::string value = getString(key);
    if (value.empty()) {
        return defaultValue;
    }
    return value[0];
}

// Кэшированные геттеры для часто используемых параметров
int ConfigManager::getScreenWidth() {
    static int cached = getInt("SCREEN_WIDTH");
    return cached;
}

int ConfigManager::getScreenHeight() {
    static int cached = getInt("SCREEN_HEIGHT");
    return cached;
}

int ConfigManager::getPlayerStartX() {
    static int cached = getInt("PLAYER_START_X");
    return cached;
}

int ConfigManager::getPlayerStartY() {
    static int cached = getInt("PLAYER_START_Y");
    return cached;
}

int ConfigManager::getPlayerWidth() {
    static int cached = getInt("PLAYER_WIDTH");
    return cached;
}

int ConfigManager::getPlayerHeight() {
    static int cached = getInt("PLAYER_HEIGHT");
    return cached;
}

int ConfigManager::getPlayerHealth() {
    static int cached = getInt("PLAYER_HEALTH");
    return cached;
}

int ConfigManager::getBulletSpeed() {
    static int cached = getInt("BULLET_SPEED");
    return cached;
}

int ConfigManager::getParryBulletSpeed() {
    static int cached = getInt("PARRY_BULLET_SPEED");
    return cached;
}

int ConfigManager::getParryDuration() {
    static int cached = getInt("PARRY_DURATION");
    return cached;
}

int ConfigManager::getDodgeDuration() {
    static int cached = getInt("DODGE_DURATION");
    return cached;
}

int ConfigManager::getDodgeDistance() {
    static int cached = getInt("DODGE_DISTANCE");
    return cached;
}

int ConfigManager::getParryRange() {
    static int cached = getInt("PARRY_RANGE");
    return cached;
}

int ConfigManager::getLevelCount() {
    static int cached = getInt("LEVEL_COUNT");
    return cached;
}

std::string ConfigManager::getLevelName(int index) {
    return getString("LEVEL_" + std::to_string(index) + "_NAME");
}

std::string ConfigManager::getLevelFile(int index) {
    return getString("LEVEL_" + std::to_string(index) + "_FILE");
}

std::string ConfigManager::getLevelDescription(int index) {
    return getString("LEVEL_" + std::to_string(index) + "_DESC");
}

// ========== АТАКА ИГРОКА (уже были упомянуты) ==========

int ConfigManager::getPlayerBulletSpeed() {
    static int cached = getInt("PLAYER_BULLET_SPEED");
    return cached;
}

int ConfigManager::getPlayerFireRate() {
    static int cached = getInt("PLAYER_FIRE_RATE");
    return cached;
}

int ConfigManager::getPlayerBulletColor() {
    static int cached = getInt("PLAYER_BULLET_COLOR");
    return cached;
}

// ========== БОСС (все параметры которые упоминались) ==========

int ConfigManager::getBossStartX() {
    static int cached = getInt("BOSS_START_X");
    return cached;
}

int ConfigManager::getBossStartY() {
    static int cached = getInt("BOSS_START_Y");
    return cached;
}

int ConfigManager::getBossWidth() {
    static int cached = getInt("BOSS_WIDTH");
    return cached;
}

int ConfigManager::getBossHeight() {
    static int cached = getInt("BOSS_HEIGHT");
    return cached;
}

int ConfigManager::getBossHealth() {
    static int cached = getInt("BOSS_HEALTH");
    return cached;
}

int ConfigManager::getBossPhase2HP() {
    static int cached = getInt("BOSS_PHASE2_HP");
    return cached;
}

int ConfigManager::getBossPhase3HP() {
    static int cached = getInt("BOSS_PHASE3_HP");
    return cached;
}

int ConfigManager::getBossBulletSpeed() {
    static int cached = getInt("BOSS_BULLET_SPEED");
    return cached;
}

int ConfigManager::getBossBulletColor() {
    static int cached = getInt("BOSS_BULLET_COLOR");
    return cached;
}

int ConfigManager::getBossBulletCooldown() {
    static int cached = getInt("BOSS_BULLET_COOLDOWN");
    return cached;
}

int ConfigManager::getBossRootWarningDuration() {
    static int cached = getInt("BOSS_ROOT_WARNING_DURATION");
    return cached;
}

int ConfigManager::getBossRootGrowDuration() {
    static int cached = getInt("BOSS_ROOT_GROW_DURATION");
    return cached;
}

int ConfigManager::getBossRootDamage() {
    static int cached = getInt("BOSS_ROOT_DAMAGE");
    return cached;
}

int ConfigManager::getBossRootCooldown() {
    static int cached = getInt("BOSS_ROOT_COOLDOWN");
    return cached;
}

int ConfigManager::getBossRootColor() {
    static int cached = getInt("BOSS_ROOT_COLOR");
    return cached;
}

void ConfigManager::reload() {
    if (instance) {
        instance->config.clear();
        instance->intCache.clear();
        instance->floatCache.clear();
        instance->loadConfigInternal();
    }
}

int ConfigManager::getViewportWidth() {
    static int cached = getInt("VIEWPORT_WIDTH");
    if (cached == 0) {
        cached = getInt("SCREEN_WIDTH"); // По умолчанию используем размер экрана
    }
    return cached;
}

int ConfigManager::getViewportHeight() {
    static int cached = getInt("VIEWPORT_HEIGHT");
    if (cached == 0) {
        cached = getInt("SCREEN_HEIGHT"); // По умолчанию используем размер экрана
    }
    return cached;
}

int ConfigManager::getCameraFollowSpeed() {
    static int cached = getInt("CAMERA_FOLLOW_SPEED");
    if (cached == 0) cached = 5; // Значение по умолчанию
    return cached;
}

int ConfigManager::getWorldWidth(const std::string& levelName) {
    if (levelName == "level1") return getInt("WORLD_WIDTH_LEVEL1");
    if (levelName == "level2-1") return getInt("WORLD_WIDTH_LEVEL2-1");
    if (levelName == "level2-2") return getInt("WORLD_WIDTH_LEVEL2-2");
    if (levelName == "level3-1") return getInt("WORLD_WIDTH_LEVEL3-1");
    if (levelName == "level3-2") return getInt("WORLD_WIDTH_LEVEL3-2");
    if (levelName == "level3-3") return getInt("WORLD_WIDTH_LEVEL3-3");

    return getInt("WORLD_WIDTH_LEVEL3-3"); // По умолчанию
}

int ConfigManager::getWorldHeight(const std::string& levelName) {
    if (levelName == "level1") return getInt("WORLD_HEIGHT_LEVEL1");
    if (levelName == "level2-1") return getInt("WORLD_HEIGHT_LEVEL2-1");
    if (levelName == "level2-2") return getInt("WORLD_HEIGHT_LEVEL2-2");
    if (levelName == "level3-1") return getInt("WORLD_HEIGHT_LEVEL3-1");
    if (levelName == "level3-2") return getInt("WORLD_HEIGHT_LEVEL3-2");
    if (levelName == "level3-3") return getInt("WORLD_HEIGHT_LEVEL3-3");

    return getInt("WORLD_HEIGHT_LEVEL3-3"); // По умолчанию
}

// ========== ЧЕКПОИНТЫ ==========
char ConfigManager::getCheckpointSymbol() {
    static char cached = getChar("CHECKPOINT_SYMBOL", '+');
    return cached;
}

int ConfigManager::getCheckpointActivationRange() {
    static int cached = getInt("CHECKPOINT_ACTIVATION_RANGE");
    if (cached == 0) cached = 2; // Значение по умолчанию
    return cached;
}

int ConfigManager::getCheckpointActiveColor() {
    static int cached = getInt("CHECKPOINT_ACTIVE_COLOR");
    if (cached == 0) cached = 10; // Ярко-зеленый по умолчанию
    return cached;
}

int ConfigManager::getCheckpointInactiveColor() {
    static int cached = getInt("CHECKPOINT_INACTIVE_COLOR");
    if (cached == 0) cached = 2; // Темно-зеленый по умолчанию
    return cached;
}