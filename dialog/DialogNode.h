#pragma once
#define _HAS_STD_BYTE 0
#include <string>
#include <vector>
#include <iostream>
#include <map>

using namespace std;

struct Condition {
    string type;
    vector<vector<string>> checks;
};

struct NpcText {
    wstring text;
    vector<Condition> conditions;
};

struct DialogOption {
    wstring text;
    string nextNode;
    vector<Condition> conditions;
    // словарь из ключ-названий скриптов, значений - словарей из ключ-названия поля, значение - модификатор
    // пример - выриант ответа делает set_bool для нескольких флагов и change_int для маны и хп
    map<string, map<string, string>> scripts; // скрипты, который выполняется при выборе этого варианта

    DialogOption(const wstring& t, 
        const string& next,
        const vector<Condition>& cond,
        const map<string, map<string, string>>& scrs);
};

class DialogNode {
private:
    string id;
    vector<NpcText> npcTexts;
    vector<DialogOption> options_all;
    mutable vector<DialogOption> options_available;
    bool allow_showing_opts_scripts = false;

public:
    DialogNode(const string& nodeId);

    // Геттеры
    string getId() const;
    vector<NpcText> getNpcText() const;
    vector<DialogOption> getOptionsAll() const;
    vector<DialogOption> getOptionsAvailable() const;

    // Методы
    void addNpcText(const wstring& text, 
        const vector<Condition>& conditions);

    void addOption(const wstring& optionText, 
        const string& nextNode, 
        const vector<Condition>& condition,
        const map<string, map<string, string>>& scripts);
    void checkOptions() const;
    void drawOptions() const;

    void display(const bool& flagEndDialog = false) const;
    bool hasAllOptions() const;
    bool hasAvailableOptions() const;
};