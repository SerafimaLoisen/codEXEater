#pragma once
#define _HAS_STD_BYTE 0
#include <string>
#include <map>
#include "DialogNode.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

static class QuestController{
private:
	static string saveFileName;

	// json помощники
	static json toJson();
	static void fromJson(const json& j);

public:
	// Поля
	static bool createdCharacter;

	static bool flagAllowShowing;
	static bool flagShowSaved;
	static string currTreeId;
	static string currNodeId;
	static map<string, int> ints;
	static map<string, bool> flags;
	static vector<string> availableDialogIDs;

	// Функции помощники
	static void initialize();
	static void loadSaveFile();
	static void saveToFile();
	static void createDefaultSaveFile();
	static void resetSavedValues();

	static void calculateOrigin();

	static bool checkConditions(vector<Condition>);
	static void logErrorConditions(vector<Condition>);

	static void ReevaluateAvailableOptions(map<string, wstring>& opts);
	static void RemoveAvailableOption(string opt);
	static void AddAvailableOption(string opt);

	// Геттеры
	static map<string, bool>* getFlags();
	static map<string, int>* getInts();


};

