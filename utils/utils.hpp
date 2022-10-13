#pragma once
#include <string>
const extern std::string NOW;
const extern std::string GLOBAL_CONFIG;
const extern std::string TEMP_SORT_FILE;
std::string timeNowString();
std::string unixTimeString();
void initGlobalConfig();