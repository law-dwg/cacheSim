#pragma once
#include "../utils/utils.hpp"
// Standard libraries
#include <stdio.h>

// #include <filesystem>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// Replacement policies
#include "fifo.hpp"
#include "lfu.hpp"
#include "lru.hpp"
#include "mru.hpp"
#include "plru.hpp"
#include "slru.hpp"

template <typename R, typename T>
struct CacheConfig
{
  // data members
  unsigned placementPolicy;
  R *replacementPolicy = nullptr; // we only care about the type
  T *dataType = nullptr;          // we only care about the type
  unsigned program;
  // strings
  std::string placementPolicyStr;
  std::string replacementPolicyStr;
  std::string dataTypeStr;
  std::string programStr;
  std::string fileout;
  std::string fileoutData;
  //
  bool suppressOut = true;
  // units in Bytes
  int cacheSize_bytes;
  int blockSize_bytes;
  int ramSize_bytes = 0;

  // units of blocks
  int setSize_blocks;   // # of blocks in set, determined at runtime/error
                        // handling, cannot set to const
  int cacheSize_blocks; // total # of blocks in cache
  // units of sets
  int cacheSize_sets; // # of sets in cache
  // units of sizeOf(T)
  int blockSize;   // # of sizeOf(T) in a block
  int cacheSize;   // # of sizeOf(T) in cache
  int ramSize = 0; // # of sizeOf(T) in RAM
  // results
  unsigned hitCount = 0;
  unsigned missCount = 0;
  std::vector<std::string> pPStrings = {
      "fully_associative",                    // pP = 0 anything can go anywhere in cache - one
                                              // large set
      "1-way_set_associative_direct_mapping", // pP = 1 each block in main
                                              // memory can go to only one
                                              // block in cache
      "2-way_set_associative",                // pP = 2 each block in main memory can go to 2
                                              // blocks in cache (1 set has two blocks)
      "3-way_set_associative",                // pP = 3 each block in main memory can go to 2
                                              // blocks in cache (1 set has three blocks)
      "4-way_set_associative",                // pP = 4 each block in main memory can go to 4
                                              // blocks in cache (1 set has four blocks)
      "5-way_set_associative", "6-way_set_associative", "7-way_set_associative",
      "8-way_set_associative"};
  std::vector<std::string> rPstrings = {"FIFO", "LFU", "LRU",
                                        "MRU", "SLRU", "PLRU"};
  std::vector<std::string> programs = {"matMul", "sort", "LSQR"};
  /* constructor */
  CacheConfig(int bS_b, int cS_b, int pP)
      : blockSize_bytes(bS_b),
        cacheSize_bytes(cS_b),
        blockSize(bS_b / sizeof(T)),
        cacheSize(cS_b / sizeof(T)),
        placementPolicy(pP)
  {
    if (cacheSize < blockSize)
      throw std::runtime_error(
          "cacheSize cannot be less than blockSize out of bounds"); // error
                                                                    // checking
    if (cacheSize_bytes % blockSize_bytes != 0)
      throw std::runtime_error(
          "cacheSize must be divisble by blocksize"); // error checking
    if (cacheSize % 2 != 0)
      throw std::runtime_error(
          "cacheSize must be divisble by 2"); // error checking
    if (blockSize_bytes % 2 != 0)
      throw std::runtime_error(
          "blocksize must be divisble by 2"); // error checking
    if (placementPolicy != 0)
    {
      if (cacheSize % placementPolicy != 0)
        throw std::runtime_error(
            "cacheSize must be divisble by set associativity"); // error
                                                                // checking
    }
    if (pP > 8)
      throw std::runtime_error(
          "placementPolicy > 8 is currently not supported "); // error checking
    replacementPolicyStr = determineRp(replacementPolicy);
    placementPolicyStr = pPStrings[placementPolicy];
    setSize_blocks =
        (pP == 0) ? (cacheSize / blockSize) : pP;       // setSize in blocks
    cacheSize_blocks = (cacheSize / blockSize);         // cacheSize in blocks
    cacheSize_sets = cacheSize_blocks / setSize_blocks; // cacheSize in sets
    printf(
        "cacheSize_bytes = %d, "
        "blockSize_bytes = %d, "
        "setSize_blocks = %d, "
        "cacheSize_blocks = %d, "
        "cacheSize_sets = %d, "
        "blockSize = %d, "
        "cacheSize = %d\n",
        cacheSize_bytes, blockSize_bytes, setSize_blocks, cacheSize_blocks,
        cacheSize_sets, blockSize, cacheSize);
  };
  // function members
  void print()
  {
    std::cout << "***CACHE-CONFIG***" << std::endl;
    std::cout << "program = " << programStr << std::endl;
    std::cout << "placement-policy = " << placementPolicyStr << std::endl;
    std::cout << "replacement-policy = " << replacementPolicyStr << std::endl;
    std::cout << "ram-size = " << ramSize << std::endl;
    std::cout << "blockSize = " << blockSize << std::endl;
    std::cout << "cacheSize = " << cacheSize_sets << " sets with "
              << setSize_blocks << " cache blocks in each (" << cacheSize_blocks
              << " blocks total)" << std::endl;
    std::cout << std::endl;
  };
  void writeOut(int g)
  {
    std::string header =
        "blockSize_bytes,cacheSize_bytes,ramSize_bytes,placementPolicy,"
        "replacementPolicy,program,hits,misses,hitRatio,filename\n";
    std::string out =
        std::to_string(blockSize_bytes) + "," +
        std::to_string(cacheSize_bytes) + "," + std::to_string(ramSize_bytes) +
        "," + placementPolicyStr + "," + replacementPolicyStr + "," +
        programStr + "," + std::to_string(hitCount) + "," +
        std::to_string(missCount) + "," +
        std::to_string(((double)hitCount / (double)(hitCount + missCount))) +
        "," + fileoutData;
    if (!fileout.empty())
    {
      std::ofstream report;
      std::stringstream reportName;
      reportName << fileout;
      report.open(reportName.str());
      report << header;
      report << (out + "\n");
      report.close();
      // if(hitCount==0) throw std::runtime_error("NO HITS?");
    }
    else
    {
      throw std::runtime_error("Cache config cannot be written\n");
    };
    std::ofstream globconf(GLOBAL_CONFIG, std::ios_base::app);
    if (globconf.is_open())
    {
      out = out + "," + std::to_string(g) + "\n";
      globconf << out;
      globconf.close();
    }
    else
    {
      std::cout << "Unable to open " << GLOBAL_CONFIG << std::endl;
    }
  };
  std::string determineRp(R *replacementPolicy)
  {
    std::string out;
    if (typeid(R) == typeid(FIFO<T>))
    {
      out = "FIFO";
    }
    else if (typeid(R) == typeid(LFU<T>))
    {
      out = "LFU";
    }
    else if (typeid(R) == typeid(LRU<T>))
    {
      out = "LRU";
    }
    else if (typeid(R) == typeid(MRU<T>))
    {
      out = "MRU";
    }
    else if (typeid(R) == typeid(SLRU<T>))
    {
      out = "SLRU";
    }
    else if (typeid(R) == typeid(PLRU<T>))
    {
      out = "PLRU";
    }
    else
    {
      throw std::runtime_error("unknown replacement policy");
    }
    return out;
  };
};

template <class R, typename T>
class Cache
{
public:
  CacheConfig<R, T> &config;
  std::vector<std::unique_ptr<R>> set; // vector of sets
  bool supressOut = true;              // tell the output to be quiet

  Cache(CacheConfig<R, T> &c, bool sO = true) : config(c)
  {
    int initHashMapSize =
        (config.ramSize / config.cacheSize_sets) /
        config
            .blockSize; // initial size of hashmap, can grow (through allocate)
    for (int i = 0; i < config.cacheSize_sets; ++i)
    { // create vector of sets
      set.emplace_back(std::make_unique<R>(config.setSize_blocks,
                                           &config.blockSize, initHashMapSize));
    };
    if (!sO)
    {
      config.print();
    };
  };
  unsigned allocate(unsigned a)
  { // increase size of ram by a
    config.ramSize += a;
    config.ramSize_bytes += sizeof(T) * a;
    for (unsigned i = 0; i < config.cacheSize_sets; ++i)
    {
      set[i]->updateRam(config.ramSize);
    };
    return config.ramSize;
  };
  bool find(int ramIdx, T &ramData)
  { // processor requests memory
    if (ramIdx > config.ramSize)
      throw std::runtime_error("ramIdx out of bounds"); // error check
    int setIndex = (ramIdx / config.blockSize) %
                   config.cacheSize_sets; // set index (cache scope)
    int tag = ((ramIdx / config.blockSize) - setIndex) /
              config.cacheSize_sets;        // tag (set scope)
    int offset = ramIdx % config.blockSize; // offset (block scope)
    // printf("numOfSets=%d, ramIdx# = %d, set# = %d, tag = %d, offset = %d\n",
    // config.cacheSize_sets, ramIdx, setIndex, tag, offset);
    return set[setIndex]->find(offset, tag, ramData);
  };
};