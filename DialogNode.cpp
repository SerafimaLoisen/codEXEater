
#include "DialogNode.h"
#include <iostream>
#include "QuestController.h"

DialogOption::DialogOption(const wstring& t, 
    const string& next,
    const vector<Condition>& cond,
    const map<string, map<string, string>>& scrs)
    : text(t), nextNode(next), conditions(cond), scripts(scrs) {
}

DialogNode::DialogNode(const string& nodeId)
    : id(nodeId){
}



string DialogNode::getId() const { return id; }
vector<NpcText> DialogNode::getNpcText() const { return npcTexts; }
vector<DialogOption> DialogNode::getOptionsAll() const { return options_all; }
vector<DialogOption> DialogNode::getOptionsAvailable() const { return options_available; }

void DialogNode::addNpcText(const wstring& text, 
    const vector<Condition>& conditions)
{
    NpcText npc_text;
    npc_text.text = text;
    npc_text.conditions = conditions;
    npcTexts.push_back(npc_text);
}

void DialogNode::addOption(const wstring& optionText, 
    const string& nextNode,
    const vector<Condition>& conditions,
    const map<string, map<string, string>>& scripts) {
    options_all.emplace_back(optionText, nextNode, conditions, scripts);
}

void DialogNode::display(const bool& flagEndDialog) const {

    wstring tmp_applicable_text = L"THIS IS A DEFAULT MESSAGE. IF YOU SEE THIS, THEN NONE OF THE CONDITIONS FOR THIS NODE WERE MET";

    for (auto text : npcTexts) {
        if (QuestController::checkConditions(text.conditions)) {
            tmp_applicable_text = text.text;
        }
    }

    wcout << tmp_applicable_text << "\n\n";


    // Проверка на наличие доступных вариантов ответа происходит на уровне выше. В дереве.
    // Метод displayCurrentNode()

    if (!flagEndDialog) {
        drawOptions();
        //wcout.flush();  // Сбросить буфер перед новым выводом

        wcout << L"\n (Выберите вариант (1-" << options_available.size() << ")): ";
    }
    else {
        wcout << L"\n (Введите что угодно для завершения диалога): ";
    }
}

void DialogNode::checkOptions() const {


    options_available.clear();


    for (size_t i = 0; i < options_all.size(); ++i) {

        if (QuestController::checkConditions(options_all[i].conditions)) {
            options_available.push_back(options_all[i]);
        }

    }

    return;
}

void DialogNode::drawOptions() const {


    for (size_t i = 0; i < options_available.size(); ++i) {

        wcout << "|-  " << i + 1 << ". " << options_available[i].text;
        if (!options_available[i].scripts.empty()) {
            wcout << "\n";
            if (allow_showing_opts_scripts){
                for (auto& script : options_available[i].scripts) {
                    cout << " [" << script.first << " : ";
                    for (auto& args : script.second) {
                        cout << " (" << args.first << " , " << args.second << ") ";
                    }
                    wcout << " ]" << "\n";
                }
            }
        }
        wcout << "\n";
    }

    return;
}

bool DialogNode::hasAllOptions() const {
    return !options_all.empty();
}
bool DialogNode::hasAvailableOptions() const {
    return !options_available.empty();
}