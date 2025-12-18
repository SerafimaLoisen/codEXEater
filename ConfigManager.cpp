#include "ConfigManager.h"
#include <fstream>
#include <sstream>
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

    std::cout << "Config loaded successfully with " << config.size() << " parameters" << std::endl;
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

// Кэшированные геттеры для часто используемых параметров

//============= ЭКРАН =============
int ConfigManager::getScreenWidth() {
    static int cached = getInt("SCREEN_WIDTH");
    return cached;
}

int ConfigManager::getScreenHeight() {
    static int cached = getInt("SCREEN_HEIGHT");
    return cached;
}

//============= ИГРОК =============

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

int ConfigManager::getPlayerColor() {
    static int cached = getInt("PLAYER_COLOR");
    return cached;
}

// ========== АТАКА ИГРОКА ==========

int ConfigManager::getPlayerCooldown() {
    static int cached = getInt("PLAYER_COOLDOWN");
    return cached;
}

int ConfigManager::getPlayerBulletColor() {
    static int cached = getInt("PLAYER_BULLET_COLOR");
    return cached;
}

//============= ТАЙМИНГИ ИГРОКА =============

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

//============= ОБЫЧНЫЕ ПУЛИ =============

int ConfigManager::getBulletSpeed() {
    static int cached = getInt("BULLET_SPEED");
    return cached;
}

int ConfigManager::getBulletColor() {
    static int cached = getInt("BULLET_COLOR");
    return cached > 0 ? cached : 7; // дефолт 7 если нет
}

int ConfigManager::getBulletDamage() {
    static int cached = getInt("BULLET_DAMAGE");
    return cached > 0 ? cached : 10; // дефолт 10
}

//============= ПАРИРУЕМЫЕ ПУЛИ =============

int ConfigManager::getParryBulletSpeed() {
    static int cached = getInt("PARRY_BULLET_SPEED");
    return cached;
}

int ConfigManager::getParryBulletColor() {
    static int cached = getInt("PARRY_BULLET_COLOR");
    return cached > 0 ? cached : 13; // дефолт 13
}

int ConfigManager::getParryBulletDamage() {
    static int cached = getInt("PARRY_BULLET_DAMAGE");
    return cached > 0 ? cached : 10; // дефолт 10
}

// ========== БОСС ==========

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
// ========== ФАЗЫ БОССА  ==========

int ConfigManager::getBossPhase2HP() {
    static int cached = getInt("BOSS_PHASE2_HP");
    return cached;
}

int ConfigManager::getBossPhase3HP() {
    static int cached = getInt("BOSS_PHASE3_HP");
    return cached;
}
// ========== ПРИСЛЕДУЮЩИЕ ПУЛИ  ==========

int ConfigManager::getFollowBulletDamage() {
    static int cached = getInt("FOLLOW_BULLET_DAMAGE");
    return cached;
}

int ConfigManager::getFollowBulletSpeed() {
    static int cached = getInt("FOLLOW_BULLET_SPEED");
    return cached;
}

int ConfigManager::getFollowBulletColor() {
    static int cached = getInt("FOLLOW_BULLET_COLOR");
    return cached;
}

int ConfigManager::getFollowBulletCooldown() {
    static int cached = getInt("FOLLOW_BULLET_COOLDOWN");
    return cached;
}

int ConfigManager::getFollowBulletFollowDuration() {
    static int cached = getInt("FOLLOW_BULLET_FOLLOW_DURATION");
    return cached;
}

// ========== КОРНИ  ==========

int ConfigManager::getBossRootWarningDuration() {
    static int cached = getInt("BOSS_ROOT_WARNING_DURATION");
    return cached;
}

int ConfigManager::getBossRootGrowDuration() {
    static int cached = getInt("BOSS_ROOT_GROW_DURATION");
    return cached;
}

int ConfigManager::getBossRootMaxLength() {
    static int cached = getInt("BOSS_ROOT_MAX_LENGTH");
    return cached > 0 ? cached : 10; // дефолт 1
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

int ConfigManager::getBossRootHealth() {
    static int cached = getInt("BOSS_ROOT_HEALTH");
    return cached > 0 ? cached : 10;
}

int ConfigManager::getBossRootCollisionCooldown() {
    static int cached = getInt("BOSS_ROOT_COLLISION_COOLDOWN");
    return cached > 0 ? cached : 10; // дефолт 10, если не задано
}

std::vector<RootSpawn> ConfigManager::getBossRootPositions() {
    std::vector<RootSpawn> positions;
    std::string value = getString("BOSS_ROOT_POSITIONS"); // пример: 10,20,Up;30,20,Down
    if (value.empty()) return positions;

    std::stringstream ss(value);
    std::string token;

    while (std::getline(ss, token, ';')) {
        std::stringstream st(token);
        std::string xStr, yStr, dirStr;

        if (!std::getline(st, xStr, ',')) continue;
        if (!std::getline(st, yStr, ',')) continue;
        if (!std::getline(st, dirStr, ',')) continue;

        int x = std::stoi(xStr);
        int y = std::stoi(yStr);
        GrowDirection dir = GrowDirection::Up;

        if (dirStr == "Up") dir = GrowDirection::Up;
        //else if (dirStr == "Down") dir = GrowDirection::Down;
        //else if (dirStr == "Left") dir = GrowDirection::Left;
        //else if (dirStr == "Right") dir = GrowDirection::Right;

        positions.push_back({ x, y, dir });
    }

    return positions;
}

//============= УРОВНИ =============

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


void ConfigManager::reload() {
    if (instance) {
        instance->config.clear();
        instance->intCache.clear();
        instance->floatCache.clear();
        instance->loadConfigInternal();
    }
}