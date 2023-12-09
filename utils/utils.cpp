#include "utils.hpp"
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

const extern std::string NOW = timeNowString();
const extern std::string GLOBAL_CONFIG =
    "results/" + NOW + "/" + NOW + "_globalconfig.csv";
const extern std::string TEMP_SORT_FILE = "sorttemp.txt";

std::string timeNowString()
{
  auto now = std::chrono::system_clock::now();
  std::time_t now_time = std::chrono::system_clock::to_time_t(now);
  tm t = *localtime(&now_time);
  std::string out;
  std::string hour = (t.tm_hour < 10) ? "0" + std::to_string(t.tm_hour)
                                      : std::to_string(t.tm_hour);
  std::string min = (t.tm_min < 10) ? "0" + std::to_string(t.tm_min)
                                    : std::to_string(t.tm_min);
  std::string mon = ((t.tm_mon + 1) < 10) ? "0" + std::to_string(t.tm_mon + 1)
                                          : std::to_string(t.tm_mon + 1);
  std::string day = (t.tm_mday < 10) ? "0" + std::to_string(t.tm_mday)
                                     : std::to_string(t.tm_mday);
  out = std::to_string(t.tm_year + 1900) + "-" + mon + "-" + day + "T" + hour +
        min;
  return out;
};

std::string unixTimeString()
{
  uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  std::string out = std::to_string(ms);
  return out;
};

void initGlobalConfig()
{
  std::string dir_results = "results/";
  std::string directoryout = dir_results + NOW;
  std::filesystem::path path_results{dir_results};
  std::filesystem::path out{directoryout};
  if (!std::filesystem::exists(path_results))
  {
    std::filesystem::create_directory(dir_results);
  };
  if (!std::filesystem::exists(out))
  {
    std::filesystem::create_directory(directoryout);
  };
  std::ofstream report(GLOBAL_CONFIG, std::ios_base::app);
  std::string input =
      "blockSize_bytes,cacheSize_bytes,ramSize_bytes,placementPolicy,"
      "replacementPolicy,program,hits,misses,hitRatio,filename,group\n";
  if (report.is_open())
  {
    report << input;
    report.close();
  }
  else
  {
    std::cout << "Unable to open file " << GLOBAL_CONFIG << std::endl;
  }
};