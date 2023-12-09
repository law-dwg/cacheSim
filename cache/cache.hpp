#pragma once
#include "../utils/utils.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <array>

// Replacement policies
#include "../policies/fifo.hpp"
#include "../policies/lfu.hpp"
#include "../policies/lru.hpp"
#include "../policies/mru.hpp"
#include "../policies/plru.hpp"
#include "../policies/slru.hpp"

// Use constexpr for constants
constexpr size_t kMaxPlacementPolicy = 8;

template <typename R, typename T>
struct CacheConfig
{
  R *replacementPolicy = nullptr; // we only care about the type
  std::array<std::string, kMaxPlacementPolicy + 1> kPPStrings = {
      // pP = 0 anything can go anywhere in cache - one large set
      "fully_associative",
      // pP = 1 each block in main memory can go to only one block in cache
      "1-way_set_associative_direct_mapping",
      // pP = 2 each block in main memory can go to 2 blocks in cache (1 set has two blocks)
      "2-way_set_associative",
      "3-way_set_associative",
      "4-way_set_associative",
      "5-way_set_associative",
      "6-way_set_associative",
      "7-way_set_associative",
      "8-way_set_associative"};
  std::vector<std::string> rPstrings = {
      "FIFO", "LFU", "LRU", "MRU", "SLRU", "PLRU"};
  std::vector<std::string> programs = {"matMul", "sort", "LSQR"};
  std::string determineReplacementPolicyStr(R *replacementPolicy) const
  {
    std::string out;
    if (typeid(R) == typeid(FIFO<T>))
      out = "FIFO";
    else if (typeid(R) == typeid(LFU<T>))
      out = "LFU";
    else if (typeid(R) == typeid(LRU<T>))
      out = "LRU";
    else if (typeid(R) == typeid(MRU<T>))
      out = "MRU";
    else if (typeid(R) == typeid(SLRU<T>))
      out = "SLRU";
    else if (typeid(R) == typeid(PLRU<T>))
      out = "PLRU";
    else
      throw std::runtime_error("unknown replacement policy");
    return out;
  };
  int ramSize_bytes = 0;
  int ramSize = 0; // # of sizeOf(T) in RAM

  bool suppressOut = true;
  unsigned hitCount = 0;
  unsigned missCount = 0;

  unsigned program;
  std::string programStr;
  std::string fileout;
  std::string fileoutData;

  int blockSize_bytes;
  int blockSize; // # of sizeOf(T) in a block
  int cacheSize_bytes;
  int cacheSize;        // # of sizeOf(T) in cache
  int cacheSize_blocks; // total # of blocks in cache
  int setSize_blocks;   // # of blocks in set, determined at runtime
  int cacheSize_sets;   // # of sets in cache
  unsigned placementPolicy;
  std::string placementPolicyStr;
  std::string replacementPolicyStr;
  CacheConfig(int bS_b, int cS_b, int pP)
      // initializer list is executed in order of declaration not order of initialization
      : blockSize_bytes(bS_b),
        blockSize(bS_b / sizeof(T)),
        cacheSize_bytes(cS_b),
        cacheSize(cS_b / sizeof(T)),
        cacheSize_blocks(cacheSize / blockSize),                  // cacheSize in blocks
        setSize_blocks((pP == 0) ? (cacheSize / blockSize) : pP), // setSize in blocks
        cacheSize_sets(cacheSize_blocks / setSize_blocks),        // cacheSize in sets
        placementPolicy(pP),
        placementPolicyStr(kPPStrings.at(placementPolicy)),
        replacementPolicyStr(determineReplacementPolicyStr(replacementPolicy))
  {
    validate();
    print();
  };

  void validate() const
  {
    if (cacheSize < blockSize)
      throw std::runtime_error("Cache size cannot be less than block size.");
    if (cacheSize_bytes % blockSize_bytes != 0)
      throw std::runtime_error("Cache size must be divisible by block size.");
    if (cacheSize % 2 != 0)
      throw std::runtime_error("Cache size must be divisible by 2.");
    if (blockSize_bytes % 2 != 0)
      throw std::runtime_error("Block size must be divisible by 2.");
    if (placementPolicy != 0 && cacheSize % placementPolicy != 0)
      throw std::runtime_error("Cache size must be divisible by set associativity.");
    if (placementPolicy > kMaxPlacementPolicy)
      throw std::runtime_error("Placement policy out of supported range.");
  }

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