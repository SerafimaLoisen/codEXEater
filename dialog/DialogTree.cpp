
#include "DialogTree.h"
#include <fstream>
#include <iostream>
#include <locale>
#include "EncodingUtils.cpp"

#pragma region PARSING FUNCTIONS

DialogTree::DialogTree()
{
    dialogActive = false;
    dialogEnded = false;
}

void DialogTree::SetTreeId(string id)
{
    thisTreeId = id;
}


string DialogTree::loadFromFile(const string& filename) {

    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Ошибка: не могу открыть файл " << filename << "\n\n";
        //return L"";
        return "";
    }

    string content; // Строка для хранения содержимого файла
    string line;        // Строка для чтения по одной строке

    // Считываем файл построчно
    while (getline(file, line)) {
        content += line + "\n"; // Добавляем строку и символ новой строки
    }

    file.close();

    // Простой поиск по ключевым словам
    size_t pos = 0;

    // Ищем character_name
    string json_tree_id = "\"tree_id\": \"";
    string json_opt_list_text = "\"opt_list_text\": \"";
    string json_charname = "\"character_name\": \"";
    string json_id = "\"id\": \"";
    string json_npctext = "\"npc_text\":";
    string json_options = "\"options\":";
    string json_options_text = "\"text\": \"";
    string json_options_next = "\"next_node\": \"";
    string json_options_scri = "\"scripts\":";
    string json_options_condition = "\"option_condition\":";

    string general_stop_sign = "\"";

    string treeID = FindValueAndPos(content, json_tree_id, pos, general_stop_sign);
    string optListText = FindValueAndPos(content, json_opt_list_text, pos, general_stop_sign);
    string tmp_str_characterName = FindValueAndPos(content, json_charname, pos, general_stop_sign);

    characterName = EncodingUtils::getWString(tmp_str_characterName);
    optionListMessage = EncodingUtils::getWString(optListText);

    // Ищем узлы диалога
    pos = 0;
    while ((pos = content.find(json_id, pos)) != string::npos) {

        // Простой парсинг ID нода
        string node_id = FindValueAndPos(content, json_id, pos, general_stop_sign);
        DialogNode* ptr_node = new DialogNode(node_id);

        size_t array_end = 0;

        // Парсинг NPC текстов (простой текст ИЛИ массив с условиями)
        size_t npc_texts_start = content.find(json_npctext, pos);
        if (npc_texts_start != string::npos) {
            size_t npc_text_value_pos = npc_texts_start + json_npctext.length();

            // Проверяем тип npc_text - простая строка или массив с условиями
            if (isSimpleText(content, npc_text_value_pos)) {
                // Парсим как простой текст без условий

                // Ищем начало строки (первую кавычку)
                size_t text_start = content.find('"', npc_text_value_pos);
                if (text_start == string::npos) break;

                // Ищем конец строки (вторую кавычку)
                size_t text_end = content.find('"', text_start + 1);
                if (text_end == string::npos) break;

                // Извлекаем текст
                string npc_text = content.substr(text_start + 1, text_end - text_start - 1);

                // Обновляем позицию для продолжения парсинга
                pos = text_end + 1;

                // conditions остается пустым - это означает, что текст всегда показывается
                vector<Condition> cond;

                ptr_node->addNpcText(EncodingUtils::getWString(npc_text), cond);
            }
            else {
                // Парсим как массив текстов с условиями
                size_t npc_texts_start = content.find(json_npctext, pos);
                if (npc_texts_start != string::npos) {

                    npc_texts_start += json_npctext.length();
                    size_t array_start = content.find('[', npc_texts_start);

                    if (array_start != string::npos) {

                        array_end = findMatchingBracket(content, array_start, '[', ']');

                        if (array_end != string::npos) {
                            string npc_texts_content = content.substr(array_start + 1, array_end - array_start - 1);

                            // Парсинг каждого текста NPC в массиве
                            size_t text_pos = 0;
                            while ((text_pos = npc_texts_content.find('{', text_pos)) != string::npos) {
                                // Парсинг записи вида {"текст", [условия]}

                                // Извлечение текста NPC
                                size_t text_start = npc_texts_content.find('"', text_pos);
                                if (text_start == string::npos) break;

                                size_t text_end = npc_texts_content.find('"', text_start + 1);
                                if (text_end == string::npos) break;

                                string npc_text = npc_texts_content.substr(text_start + 1, text_end - text_start - 1);

                                // Парсинг массива условий
                                vector<Condition> conditions;
                                size_t cond_array_pos = text_end + 1;
                                conditions = parseConditionArray(npc_texts_content, cond_array_pos);

                                // Добавление текста NPC с условиями в узел
                                ptr_node->addNpcText(EncodingUtils::getWString(npc_text), conditions);

                                text_pos = cond_array_pos;
                            }
                        }
                    }
                }
            }
        }

        // Парсинг массива вариантов ответов
        size_t options_start = content.find(json_options, pos);
        if (options_start != string::npos) {

            options_start += json_options.length();
            size_t array_start = content.find('[', options_start);

            if (array_start != string::npos) {
                array_end = findMatchingBracket(content, array_start, '[', ']');
                if (array_end != string::npos) {
                    string options_content = content.substr(array_start + 1, array_end - array_start - 1);

                    // Парсинг каждого из вариантов ответа
                    size_t opt_pos = 0;
                    while ((opt_pos = options_content.find('{', opt_pos)) != string::npos) {
                        // Парсинг варианта ответа вида {"text": "...", "next_node": "...", "option_condition": [...], "scripts": [...]}

                        // Сохраняем начальную позицию этого option для парсинга его полей
                        size_t option_start_pos = opt_pos;
                        size_t option_end_pos = findMatchingBracket(options_content, opt_pos, '{', '}');

                        // Парсим текст варианта ответа
                        string option_text = FindValueAndPos(options_content, json_options_text, opt_pos, general_stop_sign);
                        if (opt_pos == string::npos) { break; }
                        if (option_text.empty()) {
                            cerr << "Ошибка: при чтении файла " << filename << " в ноде " << node_id << "\n";
                            cerr << " НЕ УКАЗАНО ПОЛЕ ТЕКСТА ДЛЯ ВАРИАНТА ОТВЕТА \n\n";
                            break;
                        }

                        // Парсим id следующего узла
                        string option_next = FindValueAndPos(options_content, json_options_next, opt_pos, general_stop_sign);
                        if (option_next.empty()) {
                            cerr << "Ошибка: при чтении файла " << filename << " в ноде " << node_id << "\n";
                            cerr << " НЕ УКАЗАНО ПОЛЕ СЛЕДУЮЩЕГО НОДА ДЛЯ ВАРИАНТА ОТВЕТА \n\n";
                            break;
                        }
                        
                        // Парсим УСЛОВИЯ для варианта ответа (массив условий)
                        vector<Condition> option_conditions;
                        //size_t cond_search_pos = option_start_pos; // начинаем поиск с начала option
                        size_t cond_pos = options_content.find(json_options_condition, option_start_pos);
                        if (cond_pos != string::npos && cond_pos < option_end_pos) {
                            cond_pos += json_options_condition.length();
                            option_conditions = parseConditionArray(options_content, cond_pos);
                        }

                        // Парсим СКРИПТЫ для варианта ответа (массив скриптов)
                        map<string, map<string, string>> option_scripts;
                        //size_t scripts_search_pos = option_start_pos; // начинаем поиск с начала option
                        size_t scripts_pos = options_content.find(json_options_scri, option_start_pos);
                        if (scripts_pos != string::npos && scripts_pos < option_end_pos) {
                            scripts_pos += json_options_scri.length();
                            option_scripts = parseScriptsArray(options_content, scripts_pos);
                        }

                        // Добавляем вариант ответа со всеми данными
                        ptr_node->addOption(EncodingUtils::getWString(option_text), option_next, option_conditions, option_scripts);

                        // Переход к следующему варианту ответа
                        opt_pos = option_end_pos;
                        if (opt_pos == string::npos) break;
                    }
                }
            }
        }

        // Сохранение узла в карту узлов
        nodes[node_id] = ptr_node;
        pos = array_end + 1;
    }

    // Проверяем что все ноды и опции внутри них действительно куда-то ведут и вызываемы
    checkNodeCoherency();

    currentNodeId = "start";
    dialogActive = true;

    wcout << L"Успешно загружен диалог для: " << characterName << "\n";
    return treeID;
}

// Улучшенный поиск парной закрывающей скобки с учетом вложенности и кавычек
size_t DialogTree::findMatchingBracket(const string& str, size_t start, char open_brace, char close_brace) {
    int count = 1;
    bool in_quotes = false;
    char quote_char = '\0';

    for (size_t i = start+1; i < str.length(); i++) {

        // Подсчет скобок
        if (str[i] == open_brace) {
            count++;
        }
        else if (str[i] == close_brace) {
            count--;
            if (count == 0) {
                return i; // Нашли парную закрывающую скобку
            }
        }
    }
    return string::npos; // Парная скобка не найдена
}


// Проверяет, является ли npc_text простой строкой или массивом с условиями
bool DialogTree::isSimpleText(const string& content, size_t pos) {
    // Ищем следующий символ после "npc_text":
    size_t value_start = content.find_first_not_of(" \t\n\r", pos);
    if (value_start == string::npos) return true; // по умолчанию считаем простым текстом

    // Если следующий символ - двойная кавычка, это простая строка
    if (content[value_start] == '"') {
        return true;
    }
    // Если следующий символ - открывающая квадратная скобка, это массив с условиями
    else if (content[value_start] == '[') {
        return false;
    }

    return true; // по умолчанию считаем простым текстом
}


// Парсинг одного условия вида { "тип_условия" , [ ... ] }
Condition DialogTree::parseCondition(const string& condition_str, size_t& pos) {
    Condition condition;

    // --------- Сначала извлечение типа условия

    // Поиск открывающей фигурной скобки условия
    size_t cond_start = condition_str.find('{', pos);
    if (cond_start == string::npos) return condition;

    // Извлечение типа условия
    size_t type_start = condition_str.find('"', cond_start);
    if (type_start == string::npos) return condition;

    size_t type_end = condition_str.find('"', type_start + 1);
    condition.type = condition_str.substr(type_start + 1, type_end - type_start - 1);


    // --------- Теперь извлечение всего массив условий
    
    // Поиск открывающей квадратной скобки для списка проверок
    size_t bracket_start = condition_str.find('[', type_end);
    if (bracket_start == string::npos) return condition;

    // Используем улучшенный поиск закрывающей скобки
    size_t bracket_end = findMatchingBracket(condition_str, bracket_start, '[', ']');
    if (bracket_end == string::npos) return condition;

    // Извлечение содержимого между квадратными скобками
    string checks_content = condition_str.substr(bracket_start + 1, bracket_end - bracket_start - 1);


    // --------- Теперь извлечение всех конкретных условий из массива

    // Парсинг отдельных проверок внутри условия
    size_t check_pos = 0;
    while ((check_pos = checks_content.find('{', check_pos)) != string::npos) {

        // Используем улучшенный поиск для фигурных скобок
        size_t check_end = findMatchingBracket(checks_content, check_pos, '{', '}');
        if (check_end == string::npos) break;

        string check_str = checks_content.substr(check_pos + 1, check_end - check_pos - 1);


        // Парсинг параметров проверки
        vector<string> check_params;
        size_t param_pos = 0;
        while ((param_pos = check_str.find('"', param_pos)) != string::npos) {
            size_t param_end = check_str.find('"', param_pos + 1);
            if (param_end == string::npos) break;

            string param = check_str.substr(param_pos + 1, param_end - param_pos - 1);
            check_params.push_back(param);

            param_pos = param_end + 1;
        }

        if (!check_params.empty()) {
            condition.checks.push_back(check_params);
        }

        check_pos = check_end + 1;
    }

    pos = bracket_end + 1;
    return condition;
}

// Парсинг массива условий вида [ {условие1}, {условие2} ]
vector<Condition> DialogTree::parseConditionArray(const string& conditions_str, size_t& pos) {
    vector<Condition> conditions;

    // Поиск открывающей квадратной скобки массива
    size_t array_start = conditions_str.find('[', pos);
    if (array_start == string::npos) return conditions;

    // Используем улучшенный поиск закрывающей скобки
    size_t array_end = findMatchingBracket(conditions_str, array_start, '[', ']');
    if (array_end == string::npos) return conditions;

    string array_content = conditions_str.substr(array_start + 1, array_end - array_start - 1);

    // Парсинг всех условий в массиве
    size_t cond_pos = 0;
    while ((cond_pos = array_content.find('{', cond_pos)) != string::npos) {
        Condition condition = parseCondition(array_content, cond_pos);
        if (!condition.type.empty()) {
            conditions.push_back(condition);
        }
    }

    pos = array_end + 1;
    return conditions;
}

// Парсинг одного скрипта вида { "имя_скрипта" , [ параметры ] }
pair<string, map<string, string>> DialogTree::parseSingleScript(const string& script_str, size_t& pos) {
    pair<string, map<string, string>> script;

    // Ищем открывающую фигурную скобку скрипта
    size_t script_start = script_str.find('{', pos);
    if (script_start == string::npos) return script;

    // Извлекаем имя скрипта
    size_t name_start = script_str.find('"', script_start);
    if (name_start == string::npos) return script;

    size_t name_end = script_str.find('"', name_start + 1);
    if (name_end == string::npos) return script;

    script.first = script_str.substr(name_start + 1, name_end - name_start - 1);

    // Ищем открывающую квадратную скобку параметров
    size_t params_start = script_str.find('[', name_end);
    if (params_start == string::npos) return script;

    // Ищем закрывающую квадратную скобку параметров
    size_t params_end = findMatchingBracket(script_str, params_start, '[', ']');
    if (params_end == string::npos) return script;

    // Извлекаем содержимое параметров
    string params_content = script_str.substr(params_start + 1, params_end - params_start - 1);

    map<string, string> script_params;

    // Парсим параметры скрипта
    size_t param_pos = 0;
    while ((param_pos = params_content.find('{', param_pos)) != string::npos) {
        // Ищем закрывающую скобку для параметра
        size_t param_end = findMatchingBracket(params_content, param_pos, '{', '}');
        if (param_end == string::npos) break;

        // Извлекаем строку параметра
        string param_str = params_content.substr(param_pos + 1, param_end - param_pos - 1);

        // Парсим ключ и значение параметра
        size_t key_start = param_str.find('"');
        if (key_start == string::npos) break;

        size_t key_end = param_str.find('"', key_start + 1);
        if (key_end == string::npos) break;

        string param_key = param_str.substr(key_start + 1, key_end - key_start - 1);

        // Ищем значение параметра
        size_t value_start = param_str.find('"', key_end + 1);
        if (value_start == string::npos) break;

        size_t value_end = param_str.find('"', value_start + 1);
        if (value_end == string::npos) break;

        string param_value = param_str.substr(value_start + 1, value_end - value_start - 1);

        // Добавляем параметр в карту
        script_params[param_key] = param_value;

        // Перемещаем позицию для следующего параметра
        param_pos = param_end + 1;
    }

    script.second = script_params;

    // Обновляем позицию для продолжения парсинга
    pos = params_end + 1;

    return script;
}

// Парсинг массива скриптов вида [ {скрипт1}, {скрипт2} ]
map<string, map<string, string>> DialogTree::parseScriptsArray(const string& scripts_str, size_t& pos) {
    map<string, map<string, string>> scripts;

    // Ищем открывающую квадратную скобку массива скриптов
    size_t array_start = scripts_str.find('[', pos);
    if (array_start == string::npos) return scripts;

    // Ищем закрывающую квадратную скобку массива скриптов
    size_t array_end = findMatchingBracket(scripts_str, array_start, '[', ']');
    if (array_end == string::npos) return scripts;

    // Извлекаем содержимое массива скриптов
    string array_content = scripts_str.substr(array_start + 1, array_end - array_start - 1);

    // Парсим каждый скрипт в массиве
    size_t script_pos = 0;
    while ((script_pos = array_content.find('{', script_pos)) != string::npos) {
        // Парсим одиночный скрипт
        pair<string, map<string, string>> script = parseSingleScript(array_content, script_pos);

        // Добавляем скрипт в результат, если имя не пустое
        if (!script.first.empty()) {
            scripts[script.first] = script.second;
        }
    }

    // Обновляем позицию для продолжения парсинга
    pos = array_end + 1;

    return scripts;
}


#pragma endregion



void DialogTree::checkNodeCoherency() const {
    for (auto& main_node : nodes)
    {

        string tmp_main_id = main_node.second->getId();
        vector<DialogOption> tmp_main_options = main_node.second->getOptionsAll();
        vector<NpcText> tmp_main_texts = main_node.second->getNpcText();

        int tmp_opts_not_deadend_count = 0;
        bool tmp_is_referenced_flag = false;

        // Пробегаем во всем остальным нодам
        for (auto& sub_node : nodes)
        {
            string tmp_sub_id = sub_node.second->getId();
            vector<DialogOption> tmp_sub_options = sub_node.second->getOptionsAll();

            // Проверяем главные варианты ответов для подсчёта тех которые ведут куда-то 
            for (auto& main_opt : tmp_main_options)
            {
                if (main_opt.nextNode == tmp_sub_id) {
                    tmp_opts_not_deadend_count += 1;
                    // нету break т.к. может быть несколько вариантов ведущих в главный нод
                }
            }

            // Проверяем что суб варианты ответов могут ссылаться на главную ноду
            if (!tmp_is_referenced_flag) {
                for (auto& sub_opt : tmp_sub_options)
                {
                    if (sub_opt.nextNode == tmp_main_id) {
                        tmp_is_referenced_flag = true;
                        break;
                    }
                }
            }


        }

        // Ловим несоответствия
        if (!tmp_is_referenced_flag) {
            cerr << "Ошибка: узел '" << tmp_main_id << "' ни разу не был указан ни в одном из ответов дерева!" << "\n\n";
        }

        if (tmp_opts_not_deadend_count != tmp_main_options.size()) {
            cerr << "Ошибка: узел '" << tmp_main_id << "' имеет '" << tmp_main_options.size() << "' ответов, но лишь '" << tmp_opts_not_deadend_count << "' из них ведут в другие ноды!" << "\n\n";
        }

        // Проверяем и логируем ошибки в условиях нода
        for (auto& opt : tmp_main_options) {
            QuestController::logErrorConditions(opt.conditions);
        }
        for (auto& txt : tmp_main_texts) {
            QuestController::logErrorConditions(txt.conditions);
        }
    }
}

bool DialogTree::showDialog(const string& id, const bool save_progress)
{
    if (nodes.find(id) != nodes.end()) {
        currentNodeId = id;
        dialogActive = true;

        if (save_progress) {
            map<string, string> tmp_dud;
            ScriptEngine::executeScript("save_progress", tmp_dud);
        }

        displayCurrentNode();
    }
    else {
        cerr << "Ошибка: узел " << id << " для дерева " << EncodingUtils::getStringFromW(characterName) << " не найден!" << "\n\n";
        dialogActive = false;
    }
    return dialogActive;
}

string DialogTree::FindValueAndPos(const string full_string, const string search_key_string, size_t& pos, const string stop_sign) {

    string result = "";

    pos = full_string.find(search_key_string, pos);
    if (pos != string::npos) {

        pos += search_key_string.length();
        size_t end = full_string.find(stop_sign, pos);
        result = full_string.substr(pos, end - pos);

    }

    return result;
}

DialogNode* DialogTree::getCurrentNodePtr() const {
    auto it = nodes.find(currentNodeId);
    return (it != nodes.end()) ? it->second : nullptr;
    //return it->second;
}

bool DialogTree::moveToNode(const string& nodeId) {
    if (nodeId == "end") {
        endDialog();
        return true;
    }

    if (nodes.find(nodeId) != nodes.end()) {
        currentNodeId = nodeId;

        const DialogNode* node = getCurrentNodePtr();

        displayCurrentNode();


        return true;
    }

    cerr << "Ошибка: узел '" << nodeId << "' не найден!" << "\n\n";
    return false;
}

bool DialogTree::selectOption(int optionIndex) {
    const DialogNode* current = getCurrentNodePtr();
    if (!current) {
        cerr << "Ошибка: текущий узел не найден!" << "\n\n";
        return false;
    }

    if (optionIndex < 1 || optionIndex > current->getOptionsAvailable().size()) {
        cerr << "Ошибка: неверный вариант ответа!" << "\n\n";
        return false;
    }

    DialogOption selectedOption = current->getOptionsAvailable()[optionIndex - 1];


    bool tmp_ignore_node_move = false;

    bool tmp_save_flag = false;

    // Выполняем скрипт, если он есть
    if (!selectedOption.scripts.empty()) {


        for (auto& scr : selectedOption.scripts) {
            if (scr.first == "save_progress") {
                // Если это сохранение, то его следует выполнить после влияния на все остальные поля
                //tmp_save_scr = scr;
                tmp_save_flag = true;
                continue;
            }
            if ((scr.first == "switch_dialogue_to")
            || (scr.first == "switch_dialogue_to_with_save")) {
                // Если это смена диалога, то не надо загружать следующий нод
                tmp_ignore_node_move = true;
            }
            ScriptEngine::executeScript(scr.first,scr.second);
        }
    }

    if (!tmp_ignore_node_move) {
        bool temp_move_res = showDialog(selectedOption.nextNode, tmp_save_flag);
        return temp_move_res;
    }
    else {
        return true;
    }
}

void DialogTree::displayCurrentNode() const {
    system("CLS");
    //ptr_questController->drawCurrState();
    QuestController::drawCurrState();

    const DialogNode* current = getCurrentNodePtr();
    if (!current) {
        cerr << "Ошибка: не могу отобразить узел!" << "\n\n";
        return;
    }

    wcout << "\n [" << characterName << "]: ";

    current->checkOptions();
    bool tmp_endflag = false;
    // Если нет никаких вариантов ответа, то следует прекратить диалог
    if (!current->hasAvailableOptions()) {
        tmp_endflag = true;
    }

    current->display(tmp_endflag);

    if (tmp_endflag) { endDialog(); }
}

string DialogTree::getCurrentNodeId() const
{
    return currentNodeId;
}

string DialogTree::getThisTreeId() const
{
    return thisTreeId;
}

wstring DialogTree::getCharacterName() const {
    return characterName;
}

wstring DialogTree::getOptionMessage() const
{
    return optionListMessage;
}

bool DialogTree::isDialogActive() const {
    return dialogActive;
}

bool DialogTree::isDialogEnded() const {
    return dialogEnded;
}

void DialogTree::endDialog() const {
    //std::cout << "\n\n\n Диалог с " << characterName << " завершен." << "\n";
    dialogEnded = true;
}

void DialogTree::deactivateDialog() const
{
    dialogEnded = false;
    dialogActive = false;
}

DialogTree::~DialogTree()
{
    //for (auto& pair : nodes) {
    //    delete pair.second;
    //}
}