#pragma once
#include "lru.hpp"
template <typename T>
class FIFO : public LRU<T>
{
public:
  // data members

  /** constructor **/
  FIFO(int sS, int *bS, int hMS) : LRU<T>(sS, bS, hMS){};

  // function members
  std::string name() { return "FIFO"; };

  bool find(int offset, int tag, T &ramData)
  { // processor requests memory
    bool hitFlag;
    Block<T> *blkPtr;
    if (this->hit(this->hashMap[tag]))
    { // CACHE HIT
      hitFlag = true;
    }
    else
    { // CACHE MISS
      hitFlag = false;
      this->missLRU(offset, tag, ramData, blkPtr);
    }
    return hitFlag;
  };
};