#pragma once
#define _HAS_STD_BYTE 0
#include <string>
#include <map>
#include <memory>
#include "DialogNode.h"
#include "ScriptEngine.h"

using namespace std;

class DialogTree {
private:
    wstring characterName;
    wstring optionListMessage;
    map<string, DialogNode*> nodes;
    string currentNodeId;
    string thisTreeId;
    mutable bool dialogActive;
    mutable bool dialogEnded;

    // Парсер помощники
    string FindValueAndPos(const string full_string, const string search_key_string, size_t& pos, const string stop_sign);
    size_t findMatchingBracket(const string& str, size_t start, char open_brace, char close_brace);
    Condition parseCondition(const string& condition_str, size_t& pos);
    vector<Condition> parseConditionArray(const string& conditions_str, size_t& pos);
    pair<string, map<string, string>> parseSingleScript(const string& script_str, size_t& pos);
    map<string, map<string, string>> parseScriptsArray(const string& scripts_str, size_t& pos);
    bool isSimpleText(const string& content, size_t pos);

public:
    DialogTree();
    void SetTreeId(string id);

    // Загрузка и управление диалогом
    string loadFromFile(const string& filename);
    void checkNodeCoherency() const;
    bool showDialog(const string& id, const bool save_progress = false);
    DialogNode* getCurrentNodePtr() const;
    bool moveToNode(const string& nodeId);
    bool selectOption(int optionIndex);

    // Геттеры
    string getCurrentNodeId() const;
    string getThisTreeId() const;
    wstring getCharacterName() const;
    wstring getOptionMessage() const;
    bool isDialogActive() const;
    bool isDialogEnded() const;
    void endDialog() const;
    void deactivateDialog() const;

    // Утилиты
    void displayCurrentNode() const;
    ~DialogTree();
};