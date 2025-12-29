#pragma once
#define _HAS_STD_BYTE 0
#include <string>
#include <map>
#include "DialogTree.h"

using namespace std;

static class DialogSystem {
private:
    static map<string, DialogTree> dialogs;
    static DialogTree* currentDialog;

public:
    static void initialize();

    // Управление диалогами
    static void showDialog(const string& characterName, const string& id, const bool save_progress = false);
    static pair<string, wstring> loadDialog(const string& filename);
    static void processInput(int choice);

    // Геттеры
    static bool isDialogActive();
    static wstring getCurrentCharacterName();
    static void displayCurrentDialog();
    static map<string, DialogTree> getDialogs();
    static string getCurrDialogTreeID();
    static string getCurrDialogNodeID();
};