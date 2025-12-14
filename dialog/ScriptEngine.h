#pragma once
#define _HAS_STD_BYTE 0
#include <string>
#include <functional>
#include <map>
#include <iostream>
#include <algorithm>
#include "QuestController.h"

using namespace std;

static class ScriptEngine {
private:
    static map<string, function<void(map<string, string>&)>> scripts;
    static map<string, bool>* ptr_flags;
    static map<string, int>* ptr_ints;

public:
    static void initialize();

    // Работа со скриптами
    static void registerScript(const string& scriptName, function<void(map<string, string>&)>);
    static void executeScript(const string& scriptName, map<string, string>& params);

    // Доп функции помощники
};