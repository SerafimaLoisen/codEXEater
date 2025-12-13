#pragma once

#define _HAS_STD_BYTE 0
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS 1

#include <string>
#include <locale>
#include <codecvt>
#include <windows.h>


using namespace std;

class EncodingUtils {
public:
    static void setupConsoleEncoding() {

        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        setlocale(LC_ALL, "Russian");
    }

    static wstring getWString(string str) {
        if (str.empty()) return L"";

        str = replaceAll(str, "\\n", "\n");

        int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        wstring result(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &result[0], size);
        return result;
    }

    static string getStringFromW(wstring wstr) {
        if (wstr.empty()) return "";
        int size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string result(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &result[0], size, NULL, NULL);
        return result;
    }

    static std::string replaceAll(std::string str, const std::string& find_str, const std::string& replace_str) {

        size_t index = 0;
        while ((index = str.find(find_str, index)) != std::string::npos) {
            str.replace(index, find_str.length(), replace_str);
            // Advance index to the character after the newly inserted newline to continue the search
            index += replace_str.length();
        }

        return str;
    }
};