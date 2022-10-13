#pragma once
#include "lru.hpp"
template <typename T>
class MRU : public LRU<T> {
 public:
  /** MRU cache replacement policy
   * MRU is the next block to be evicted
   * LRU is the last to be evicted
   * data members
   **/

  /** constructors **/
  MRU(int sS, int *bS, int hMS) : LRU<T>(sS, bS, hMS){};

  // function members
  std::string name() { return "MRU"; };

  void missMRU(int offset, int tag, T &ramData, Block<T> *blkPtr) {
    if (!this->dll.full) {                            // if cache not full
      blkPtr = &this->dll.blocks[this->dll.counter];  // new node
      this->increment();
    } else {                    // if cache full
      blkPtr = this->dll.head;  // MRU
      this->lastEvicted = *blkPtr;
      this->dll.remove(blkPtr);
      blkPtr->reset();  // clear block data & hashmap
    }
    this->updateNewBlock(offset, tag, ramData, blkPtr);
    this->dll.insertBeginning(blkPtr);
  };

  bool find(int offset, int tag, T &ramData) {  // processor requests memory
    bool hitFlag;
    Block<T> *blkPtr;
    if (this->hit(this->hashMap[tag])) {  // CACHE HIT
      hitFlag = true;                     // hit
      if (this->dll.size != 1) {  // if direct mapping, you dont need to move
                                  // anything, it stays where it is
        blkPtr = this->hashMap[tag];  // set the working pointer
        this->hitLRU(tag, blkPtr);    // stays same
      }
    } else {  // CACHE MISS
      hitFlag = false;
      missMRU(offset, tag, ramData, blkPtr);
    }
    // see lru for debugging printout examples
    return hitFlag;
  };
};