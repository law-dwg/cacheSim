#pragma once
#include <stdio.h>
#include <stdlib.h>

#include <filesystem>
//#include <experimental/filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "../cacheReplacementPolicies/cache.hpp"
#include "utils.hpp"

template <class Q>
struct Logger {
  // data memebers
  Q &cache;
  std::string fileout_data;
  std::string fileout_config;
  /** constructor **/
  Logger(Q &c) : cache(c) {
    /** Prepare report */
    std::ofstream report;
    std::stringstream reportName;
    std::string directoryout = "results/" + NOW + "/" + cache.config.programStr;
    std::string directoryout_config = directoryout + "/config";
    std::string directoryout_data = directoryout + "/data";

    std::string filename = cache.config.replacementPolicyStr + "_A" +
                           std::to_string(cache.config.placementPolicy) + "_B" +
                           std::to_string(cache.config.blockSize) + "_C" +
                           std::to_string(cache.config.cacheSize_blocks) + "_" +
                           unixTimeString();
    fileout_data = directoryout_data + "/" + filename + ".csv";
    fileout_config = directoryout_config + "/" + filename + "_config.csv";

    std::filesystem::path outConfig{directoryout_config};
    std::filesystem::path outData{directoryout_data};
    std::filesystem::file_status s = std::filesystem::file_status{};

    if (!std::filesystem::exists(outData)) {
      std::filesystem::create_directories(directoryout_data);
    };
    if (!std::filesystem::exists(outConfig)) {
      std::filesystem::create_directories(directoryout_config);
    };

    reportName << fileout_data;
    report.open(reportName.str());
    report << "data\n";
    report.close();
  };

  // operators
  bool operator()(std::string s) {
    bool success = writeToCSV(fileout_data, s);
    if (!success) throw std::runtime_error("cannot write to logfile");
    return success;
  };

  // function members
  bool writeToCSV(std::string dest, std::string input) {
    std::ofstream report(dest, std::ios_base::app);
    bool success = false;
    if (report.is_open()) {
      report << input;
      report.close();
      success = true;
    } else {
      std::cout << "Unable to open file";
      success = false;
    }
    return success;
  };
  void demo_exists(
      const std::filesystem::path &p,
      std::filesystem::file_status s = std::filesystem::file_status{}) {
    bool out;
    if (std::filesystem::status_known(s) ? std::filesystem::exists(s)
                                         : std::filesystem::exists(p)) {
      out = true;
    } else {
      out = false;
    }
  };
};