
#include "ScriptEngine.h"
#include "DialogSystem.h"

using namespace std;

map<string, function<void(map<string, string>&)>> ScriptEngine::scripts;
map<string, bool>* ScriptEngine::ptr_flags;
map<string, int>* ScriptEngine::ptr_ints;

void ScriptEngine::initialize()
{
    ptr_flags = QuestController::getFlags();
    ptr_ints = QuestController::getInts();

    // set bool flag to something
    registerScript("set_bool", [](const map<string, string>& args) {
        for (const auto& arg : args) {
            // flags contain given flag name
            if (ptr_flags->count(arg.first)) {
                bool tmp_value = (arg.second == "true" || arg.second == "1");
                ptr_flags->at(arg.first) = tmp_value;
            }
        }
        });

    // script to CHANGE value BY something
    registerScript("change_int", [](const map<string, string>& args) {
        for (const auto& arg : args) {
            // flags contain given int name
            if (ptr_ints->count(arg.first)) {

                int tmp_value = stoi(arg.second);

                ptr_ints->at(arg.first) += tmp_value;

                checkSpecialInts(arg.first);
            }
        }
        });

    // script to SWITCH current dialog tree to some other 
    registerScript("switch_dialogue_to", [](const map<string, string>& args) {
        for (const auto& arg : args) {

            DialogSystem::showDialog(arg.first, arg.second, false);

        }
        });

    // script to SWITCH current dialog tree to some other 
    registerScript("switch_dialogue_to_with_save", [](const map<string, string>& args) {
        for (const auto& arg : args) {

            DialogSystem::showDialog(arg.first, arg.second, true);

        }
        });

    // script to set dialog to be available or not
    registerScript("set_dialog_available", [](const map<string, string>& args) {
        for (const auto& arg : args) {
            bool tmp_value = (arg.second == "true" || arg.second == "1");

            if (tmp_value) {
                QuestController::AddAvailableOption(arg.first);
            }
            else {
                QuestController::RemoveAvailableOption(arg.first);
            }

        }
        });

    // script to start game
    registerScript("start_game", [](const map<string, string>& args) {
        for (const auto& arg : args) {

            // no args should be provided

        }
        });

    // script to start level
    registerScript("start_level", [](const map<string, string>& args) {
        for (const auto& arg : args) {

            // arg.first  -  name of the level to be loaded     arg.second -  is not used

        }
        });

}

void ScriptEngine::registerScript(const string& scriptName, function<void(map<string, string>&)> script)
{
    scripts[scriptName] = script;
}

void ScriptEngine::executeScript(const string& scriptName, map<string, string>& params) {
    if (scripts.count(scriptName)) {
        auto& ptr_script = scripts.at(scriptName);
        ptr_script(params);
    }
    else {
        cerr << "Ошибка: попытка вызвать скрипт " << scriptName << " провалилась. Проверьте диалоги и внутренний код на наличие этого скрипта\n\n";
    }
}