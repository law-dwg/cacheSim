#pragma once
#include <algorithm>
#include <fstream>

#include "../utils/wrappers.hpp"
#ifndef SORT_HPP_
#define SORT_HPP_

template <typename C>
int sortMain(C &config);

#include "sort.hpp"

#endif

// R = replacement policy type, T = data type
template <typename R, typename T>
struct sortWrapper {
  CacheConfig<R, T> &config;
  sortWrapper(CacheConfig<R, T> &c, int N) : config(c) {
    config.program = 1;
    config.programStr = config.programs[config.program];
    config.print();
    int out = sortMain<R, T>(config);
    // config.writeOut();
  };
  void run() {
    config.program = 1;
    config.programStr = config.programs[config.program];
    config.print();
    int out = sortMain<R, T>(config);
    // config.writeOut();
  };
};

template <typename T>
void writeArray(int rangeLow, int rangeHigh, int dim, std::string dest) {
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<double> dist(rangeLow, rangeHigh);
  std::vector<T> arr;
  for (int i = 0; i < dim; ++i) {
    arr.push_back(dist(mt));
  };
  std::ofstream destStream(dest);
  if (destStream.is_open()) {
    for (int i = 0; i < dim; i++) {
      destStream << arr[i] << "\n";
    }
    destStream.close();
  } else {
    std::cout << "Unable to open file";
  }
};

template <typename T>
void readArray(std::string dest, std::vector<T> &out) {
  std::ifstream file(dest);
  if (file.good()) {
    double i = 0;
    while (file >> i) {
      out.push_back(i);
    }
    file.close();
  } else {
    std::runtime_error("error!");
  }
};

void deleteArray(std::string dest) { std::remove(dest.c_str()); };

// R = replacement policy type, T = data type
template <typename R, typename T>
int sortMain(CacheConfig<R, T> &config) {
  typedef T dataType_t;
  typedef R replacementPolicy_t;
  std::vector<T> values_to_sort;
  readArray(TEMP_SORT_FILE,
            values_to_sort);  // requires writeArray to first be called
  CacheWrapper<R, T, std::vector<T>> L1(config);
  std::cout << "vectsize " << values_to_sort.size() << std::endl;
  auto values_to_sort_wrapper =
      L1.allocate(values_to_sort.size(), values_to_sort);
  // bubble sort
  dataType_t temp;
  for (int i = 0; i < values_to_sort.size(); ++i) {
    for (int j = i + 1; j < values_to_sort.size(); ++j) {
      if (values_to_sort_wrapper[j] < values_to_sort_wrapper[i]) {
        temp = values_to_sort_wrapper[i];
        values_to_sort[i] = values_to_sort_wrapper[j];
        values_to_sort[j] = temp;
      };
    };
  };
  return 0;
};