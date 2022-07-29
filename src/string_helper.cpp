//
// Created by jj on 18/07/22.
//

#include "../include/string_helper.h"

bool string_startswith(const std::string& main_str, const std::string& comp_str) {
    for (int i = 0; i < comp_str.length(); ++i) {
        if (comp_str[i] != main_str[i])
            return false;
    }
    return true;
}

std::vector<std::string> string_split(const std::string& s, char token) {
    std::vector<std::string> v;
    std::string temp;
    for (char curr_char: s) {
        if (curr_char == '\r' or curr_char == '\n') {
            continue;
        }
        if (curr_char == token) {
            v.push_back(temp);
            temp = "";
        } else {
            temp += curr_char;
        }
    }

    if (!temp.empty()) {
        v.push_back(temp);
    }
    return v;
}

std::vector<std::string> string_file_msg_split(const std::string& s, char token, int occur) {
    std::vector<std::string> v;
    std::string temp;
    int count = 0;
    for (char curr_char: s) {
        if (curr_char == '\r' or curr_char == '\n') {
            continue;
        }
        if (curr_char == token and count != occur) {
            count++;
            v.push_back(temp);
            temp = "";
        } else {
            temp += curr_char;
        }
    }
    if (!temp.empty()) {
        v.push_back(temp);
    }
    return v;
}

std::string string_strip(std::string& str) {
    int start, end;
    for (int i = 0; i < str.length(); ++i) {
        if (str[i] == ' ')
            continue;
        start = i;
        break;
    }
    for (int i = (int) str.length(); i >= 0; --i) {
        if (str[i] == ' ' or str[i] == '\r' or str[i] == '\n' or str[i] == '\t' or str[i] == '\0')
            continue;
        end = i;
        break;
    }
    return str.substr(start, end - start + 1);
}

bool string_contains(const std::string& str1, const std::string& str2){
    return str1.find(str2) != std::string::npos;
}