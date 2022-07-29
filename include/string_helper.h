//
// Created by jj on 22/07/22.
//

#ifndef BBSERVER2_STRING_HELPER_H
#define BBSERVER2_STRING_HELPER_H

#include <string>
#include <vector>

extern bool string_startswith(const std::string &main_str, const std::string &comp_str);

extern std::vector<std::string> string_split(const std::string &s, char token);

extern std::vector<std::string> string_file_msg_split(const std::string &s, char token, int occur);

extern std::string string_strip(std::string &str);

extern bool string_contains(const std::string &str1, const std::string &str2);

#endif //BBSERVER2_STRING_HELPER_H
