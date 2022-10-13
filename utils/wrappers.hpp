#pragma once
#include <chrono>
#include <ctime>
#include <string>
#include <vector>

#include "../cacheReplacementPolicies/cache.hpp"
#include "logger.hpp"
#include "utils.hpp"

template <typename X>
struct LoggerWrapper {
  X &cache;       // one cache
  Logger<X> log;  // one log per cache
  LoggerWrapper(X &c) : cache(c), log(c) {
    cache.config.fileoutData = log.fileout_data;
    cache.config.fileout = log.fileout_config;
  };
  std::string operator()(std::string s) {
    // std::chrono::milliseconds ms = std::chrono::duration_cast<
    // std::chrono::milliseconds >(
    // std::chrono::system_clock::now().time_since_epoch());
    // std::string out = s +","+std::to_string(ms.count()) + "\n";
    // std::cout<<out<<std::endl;
    log(s + "\n");
    return s;
  };
};

template <typename C, typename T, typename V>
struct DataWrapper {
  int offset;
  V &dataset;             // reference to dataset
  C &cache;               // reference to cache in cachewrapper
  LoggerWrapper<C> &log;  // reference to logger in cachewrapper
  DataWrapper(int o, V &ds, C &c, LoggerWrapper<C> &l)
      : offset(o), dataset(ds), log(l), cache(c){};
  T operator[](int lIdx) {
    unsigned gIdx = offset + lIdx;
    bool temp = cache.find(gIdx, dataset[lIdx]);
    std::string out = (temp) ? "1" : "0";
    // log(out);
    if (temp) {
      ++cache.config.hitCount;
    } else {
      ++cache.config.missCount;
    }
    log(out);
    return dataset[lIdx];
  };
};

template <typename R, typename T, typename V>
struct CacheWrapper {
  typedef Cache<R, T> CacheType;
  CacheType cache;               // one cache
  LoggerWrapper<CacheType> log;  // one log per cache
  CacheWrapper(CacheConfig<R, T> &c) : cache(c), log(cache){};
  DataWrapper<CacheType, T, V> allocate(unsigned a, V &v) {
    unsigned tempBaseLine = cache.config.ramSize;
    DataWrapper<CacheType, T, V> data(cache.config.ramSize, v, cache, log);
    cache.allocate(a);
    return data;
  };
};