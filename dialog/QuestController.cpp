
#include "QuestController.h"
#include <iostream>
#include <fstream>
#include "EncodingUtils.cpp"
#include "DialogSystem.h"

// --------------------- ОПРЕДЕЛЕНИЕ СТАТИЧЕСКИХ ПОЛЕЙ
// 
// ЕСЛИ ЭТОГО НЕ БУДЕТ, ТО БУДЕТ РУГАТЬСЯ ОШИБКОЙ LNK2001
string QuestController::nextLevelID = "";
string QuestController::nextDialogID = "";
string QuestController::saveFileName = "save.json";
string QuestController::currNodeId = "";
string QuestController::currTreeId = "";
vector<string> QuestController::availableDialogIDs;

bool QuestController::flagAllowShowing = true;
bool QuestController::flagShowSaved = false;

// THIS IS ALL OF THE BASELINE PARAMTRES
map<string, int> QuestController::ints = {};
map<string, bool> QuestController::flags = {};


// --------------------- ФУНКЦИИ

void QuestController::initialize() {
	// Пытаемся загрузить файл, если не существует - создаем default
	ifstream file(saveFileName);
	if (file.good()) {
		file.close();
		loadSaveFile();
	}
	else {
		createDefaultSaveFile();
	}
}

// in future need tp add here reading of save file
void QuestController::loadSaveFile()
{
	try {
		// ifstream - input file stream (для ввода)
		ifstream file(saveFileName);
		if (!file.is_open()) {
			cerr << "Ошибка загрузки сохранения: не могу открыть файл " << saveFileName << "\n";
			return;
		}

		json j;
		file >> j;
		file.close();

		fromJson(j);
		wcout << L"Файл сохранения успешно загружен!" << "\n\n";

	}
	catch (const exception& e) {
		cerr << "Ошибка загрузки файла сохранения: " << e.what() << "\n";
		// Если файл поврежден, создаем новый с default значениями
		createDefaultSaveFile();
	}
}

void QuestController::saveToFile() {
	try {
		// ofstream - output file stream (для вывода)
		ofstream file(saveFileName);
		if (!file.is_open()) {
			cerr << "Ошибка: не могу создать файл сохранения " << saveFileName << "\n";
			return;
		}

		json j = toJson();
		file << j.dump(4);  // 4 - отступ для красивого форматирования
		file.close();

		wcout << L"Игра сохранена в файл " << EncodingUtils::getWString(saveFileName) << "\n\n";

	}
	catch (const exception& e) {
		cerr << "Ошибка сохранения: " << e.what() << "\n";
	}
}

void QuestController::createDefaultSaveFile() {

	wcout << L"Создаю новый файл сохранений, возможно старый не был обнаружен \n\n";


	resetSavedValues();


	// Сохраняем default значения в файл
	saveToFile();
	wcout << L"Создан новый файл сохранения с default значениями!" << "\n\n";
}

void QuestController::resetSavedValues()
{
	// Заполняем значениями по умолчанию
	currTreeId = "";
	currNodeId = "";

	ints = {
		{"points",0},
		{"feature_fix",0},
	};

	flags = {
		// All of the origin flags
		{"fixedFeatureWithDialog",0},
	};
}

// ---- Парс json помощники

json QuestController::toJson() {
	json j;

	j["currTreeId"] = currTreeId;
	j["currNodeId"] = currNodeId;
	j["availableDialogIDs"] = availableDialogIDs;
	j["ints"] = ints;
	j["flags"] = flags;

	return j;
}

void QuestController::fromJson(const json& j) {
	try {

		if (j.contains("currTreeId")) {
			currTreeId = j["currTreeId"];
		}

		if (j.contains("currNodeId")) {
			currNodeId = j["currNodeId"];
		}

		if (j.contains("availableDialogIDs")) {
			availableDialogIDs = j["availableDialogIDs"];
		}

		if (j.contains("ints")) {
			ints = j["ints"].get<map<string, int>>();
		}

		if (j.contains("flags")) {
			flags = j["flags"].get<map<string, bool>>();
		}

	}
	catch (const exception& e) {
		cerr << "Ошибка парсинга JSON: " << e.what() << "\n";
		throw; // Перебрасываем исключение дальше
	}
}


// ---- Остальные функции

bool QuestController::checkConditions(vector<Condition> conditions)
{
	bool tmp_result = true;

	for (auto& cond : conditions) {

		if (cond.type == "default") {
			continue;
		}
		else if (cond.type == "int_is") {
			for (auto& params : cond.checks) {
				//{ "mana", ">", "1" }
				string field = params[0];
				string judge = params[1];
				string value = params[2];

				// Предпоиск такого поля в словаре
				if (ints.find(field) != ints.end()) {
					tmp_result = false;
					break;
				}

				// Преобразование полей
				int act_value = stoi(value);

				// Сама проверка
				if (judge == ">") {
					if (!(ints.at(field) > act_value)) {
						tmp_result = false;
						break;
					}
				}
				else if (judge == "<") {
					if (!(ints.at(field) < act_value)) {
						tmp_result = false;
						break;
					}
				}
				else if (judge == "=") {
					if (!(ints.at(field) == act_value)) {
						tmp_result = false;
						break;
					}
				}
				else if (judge == ">=") {
					if (!(ints.at(field) >= act_value)) {
						tmp_result = false;
						break;
					}
				}
				else if (judge == "<=") {
					if (!(ints.at(field) <= act_value)) {
						tmp_result = false;
						break;
					}
				}
				else if (judge == "!=") {
					if (!(ints.at(field) != act_value)) {
						tmp_result = false;
						break;
					}
				}
				else {
					tmp_result = false;
					break;
				}

			}
		}
		else if (cond.type == "bool_is") {
			for (auto& params : cond.checks) {
				//{ "canSpellFireball", "false" }
				string field = params[0];
				string value = params[1];

				// Предпоиск такого поля в словаре
				if (flags.find(field) != flags.end()) {
					tmp_result = false;
					break;
				}

				// Преобразование полей
				bool act_value = false;
				if (value == "1" || value == "true") {
					act_value = true;
				}

				// Сама проверка
				if (flags.at(field) != act_value) {
					tmp_result = false;
					break;
				}
			}
		}
		else {
			tmp_result = false;
			break;
		}
	}

	return tmp_result;
}

void QuestController::logErrorConditions(vector<Condition> conditions)
{
	for (auto& cond : conditions) {

		if (cond.type == "default") {
			continue;
		}
		else if (cond.type == "int_is") {
			for (auto& params : cond.checks) {
				//{ "mana", ">", "1" }
				string field = params[0];
				string judge = params[1];
				string value = params[2];

				if (ints.find(field) != ints.end()) {
					cerr << "Ошибка: при проверке условий было обнаружено несуществующее поле " << field << "\n";
					cerr << "Проверьте ваши файлы, возможные поля перечислены в словаре файла QuestController.cpp \n\n";
				}

				int act_value = stoi(value);

				if (!(judge == ">"
					|| judge == "<"
					|| judge == "="
					|| judge == ">="
					|| judge == "<="
					|| judge == "!=")) {
					cerr << "Ошибка: при проверке условий было обнаружен неверный формат сравнителя - " << judge << "\n";
					cerr << "Проверьте ваши файлы, сравнители могут быть \">\", \"<\", \"=\", \">=\", \"<=\", \"!=\"  \n\n";
				}

			}
		}
		else if (cond.type == "bool_is") {
			for (auto& params : cond.checks) {
				//{ "canSpellFireball", "false" }
				string field = params[0];
				string value = params[1];

				if (flags.find(field) != flags.end()) {
					cerr << "Ошибка: при проверке условий было обнаружено несуществующее поле " << field << "\n";
					cerr << "Проверьте ваши файлы, возможные поля перечислены в словаре файла QuestController.cpp \n\n";
				}

				if (!(value == "1" || value == "true"
					|| value == "0" || value == "false")) {
					cerr << "Ошибка: при проверке условий было обнаружен неверный формат флага сравнения " << value << "\n";
					cerr << "Проверьте ваши файлы, флаги сравнения могут быть \"1\" , \"0\" или \"true\" , \"false\"\n\n";
				}
			}
		}
		else {
			cerr << "Ошибка: при проверке условий было обнаружено непонятное условие " << cond.type << "\n";
			cerr << "Проверьте ваши файлы, условия могут быть \"int_is\", \"bool_is\" или \"default\"  \n\n";
		}
	}
}

void QuestController::ReevaluateAvailableOptions(map<string, wstring>& opts)
{

	auto dialogs = DialogSystem::getDialogs();

	for (auto& d : dialogs) {
		// Ключа нет в opts
		if (opts.find(d.first) == opts.end()) {
			// Но есть в availableDialogIDs
			if (find(availableDialogIDs.begin(),
				availableDialogIDs.end(),
				d.first) != availableDialogIDs.end()) {

				// Значит нужно добавить в opts
				opts.emplace(d.first, d.second.getOptionMessage());
			}
		}
		// Ключ есть в opts
		else {
			// Но больше нет в availableDialogIDs
			if (find(availableDialogIDs.begin(),
				availableDialogIDs.end(),
				d.first) == availableDialogIDs.end()) {

				// Значит нужно удалить из opts
				opts.erase(d.first);
			}
		}
	}



	return;
}

void QuestController::RemoveAvailableOption(string opt)
{
	availableDialogIDs.erase
	(remove(availableDialogIDs.begin(), 
		availableDialogIDs.end(), opt),
		availableDialogIDs.end());
}

void QuestController::AddAvailableOption(string opt)
{
	availableDialogIDs.push_back(opt);
}

map<string, bool>* QuestController::getFlags()
{
	return &flags;
}

map<string, int>* QuestController::getInts()
{
	return &ints;
}

void QuestController::setNextLevel(string id)
{
	nextLevelID = id;
}

string QuestController::getNextLevel()
{
	return nextLevelID;
}

void QuestController::setNextDialog(string id)
{
	nextDialogID = id;
}

string QuestController::getNextDialog()
{
	return nextDialogID;
}
