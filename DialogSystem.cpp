
#include "DialogSystem.h"
#include <iostream>
#include "EncodingUtils.cpp"


map<string, DialogTree> DialogSystem::dialogs;
DialogTree* DialogSystem::currentDialog;

void DialogSystem::initialize() {
    currentDialog = nullptr;
}

pair<string,wstring> DialogSystem::loadDialog(const string& filename) {
    auto tree = new DialogTree();
    string treeID = tree->loadFromFile(filename);
    wstring treeOptText = tree->getOptionMessage();

    tree->SetTreeId(treeID);

    pair<string, wstring> res = { treeID, treeOptText };

    dialogs[treeID] = *tree;

    return res;
}

void DialogSystem::showDialog(const string& treeId, const string& id, const bool save_progress)
{
    if (dialogs.find(treeId) != dialogs.end()) {
        if (currentDialog != nullptr) {
            currentDialog->deactivateDialog();
        }
        currentDialog = &dialogs[treeId];
        currentDialog->showDialog(id, save_progress);
    }
    else {
        wcout << L"Диалог с " << EncodingUtils::getWString(treeId) << L" не найден!" << "\n";
    }
}

void DialogSystem::processInput(int choice) {
    if (currentDialog) {
        if (!currentDialog->isDialogEnded()) {
            if (!currentDialog->selectOption(choice)) {
                wcout << L"Неверный выбор. Попробуйте снова." << "\n";
            }
        }
        else {
            currentDialog->deactivateDialog();
        }
    }
}

bool DialogSystem::isDialogActive() {
    return currentDialog && currentDialog->isDialogActive();
}

wstring DialogSystem::getCurrentCharacterName() {
    return currentDialog ? currentDialog->getCharacterName() : L"";
}

void DialogSystem::displayCurrentDialog() {
    if (currentDialog) {
        currentDialog->displayCurrentNode();
    }
}
map<string, DialogTree> DialogSystem::getDialogs()
{
    return dialogs;
}
string DialogSystem::getCurrDialogTreeID()
{
    string res = "";
    if (currentDialog) {
        res = currentDialog->getThisTreeId();
    }
    return res;
}
string DialogSystem::getCurrDialogNodeID()
{
    string res = "";
    if (currentDialog) {
        res = currentDialog->getCurrentNodeId();
    }
    return res;
}