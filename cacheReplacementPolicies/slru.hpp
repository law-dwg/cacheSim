#pragma once
#include <cstddef>
#include <stack>

#include "lru.hpp"

template <typename T>
class SLRU : public LRU<T> {
 public:
  // data members
  LRU<T> protectedLRU;
  std::stack<unsigned>
      freeIdx;  // used to track the free indices in block vector

  /** constructor **/
  SLRU(int sS, int *bS, int hMS)
      : LRU<T>(sS / 2, bS, hMS), protectedLRU(sS / 2, bS, hMS) {
    if (sS % 2 != 0)
      throw std::runtime_error(
          "SLRU sets must be divisible by 2, please adjust");
    printf("CREATING SLRU SET W/ #ofblocks = %d, blocksize = %d\n",
           this->dll.size, *this->blockSize);
    printf("protected has %d blocks\nprobational has %d blocks\n",
           protectedLRU.dll.size, this->dll.size);
  };

  // function members
  std::string name() { return "SLRU"; };

  void hitSLRU(int offset, int tag, T &ramData,
               Block<T> *blkPtr) {            // HIT IN PROBATIONAL
    bool protFull = protectedLRU.dll.full;    // save for later if a block is
                                              // being inserted into probational
    protectedLRU.find(offset, tag, ramData);  // add it to protected

    this->dll.remove(blkPtr);  // evict from probational
    freeIdx.push((unsigned)(this->hashMap[tag] -
                            &this->dll.blocks[0]));  // add the free index after
                                                     // eviction to stack
    this->decrement();                               // reduce counter
    blkPtr->reset();                                 // clear block & hashmap

    if (protFull) {  // if a block was eliminated from protected (before new
                     // block was added)
      int tempTag = (protectedLRU.lastEvicted.tag -
                     &protectedLRU.hashMap[0]);  // find the tag in main mem of
                                                 // block evicted from protected
      double temp = 0;
      this->missSLRU(0, tempTag, temp,
                     blkPtr);  // add to head of the not full prob
    }
  };

  void missSLRU(int offset, int tag, T &ramData, Block<T> *blkPtr) {
    if (!this->dll.full) {  // if prob not full
      unsigned
          tempIdx;  // we have to determine which index of dll vector to use
      if (!freeIdx.empty()) {         // is the stack not empty
        tempIdx = freeIdx.top();      // we use the free index at top of stack
        freeIdx.pop();                // pop off top
      } else {                        // if nothing in stack
        tempIdx = this->dll.counter;  // use counter index
      };
      blkPtr = &this->dll.blocks[tempIdx];  // new node
      this->increment();                    // add to counter
    } else {                                // if prob is full we use tail (LRU)
      blkPtr = this->dll.tail;              // probational LRU
      this->lastEvicted = *blkPtr;  // create copy of what was last evicted
      this->dll.remove(blkPtr);     // remove from list
      blkPtr->reset();              // clear block data & hashmap
    }
    this->updateNewBlock(offset, tag, ramData, blkPtr);  // fill block with data
    this->dll.insertBeginning(blkPtr);  // shift to head of prob
  };

  bool find(int offset, int tag, T &ramData) {  // processor requests memory
    bool hitFlag;
    Block<T> *blkPtr;
    if (this->hit(this->hashMap[tag])) {  // HIT IN PROBATIONAL
      hitFlag = true;
      blkPtr = this->hashMap[tag];
      hitSLRU(offset, tag, ramData, blkPtr);
    } else if (protectedLRU.hit(
                   protectedLRU.hashMap[tag])) {  // HIT IN PROTECTED
      hitFlag = protectedLRU.find(offset, tag, ramData);
    } else {  // MISS
      hitFlag = false;
      this->missSLRU(offset, tag, ramData, blkPtr);
    };

    // (hitFlag) ? printf("hit\n") : printf("miss\n");
    // for (int i = 0; i < this->hashMap.size(); ++i) {
    //   // if (protect.hashMap[i] != nullptr) {
    //   printf("protect address %i %p\n", i, protect.hashMap[i]);
    //   //}
    // };
    // printf("\n");
    // for (int i = 0; i < this->hashMap.size(); ++i) {
    //   // if (this->hashMap[i] != nullptr) {
    //   printf("probational address %i %p\n", i, this->hashMap[i]);
    //   //}
    // };
    // printf("\n");
    // printf("prot counter %i protfull = %d\n", protect.dll.counter,
    // protect.dll.full); printf("prob counter %i probfull = %d\n",
    // this->dll.counter, this->dll.full); printf("prot mru %p\n",
    // protect.dll.head); printf("prot lru %p\n", protect.dll.tail);
    // printf("prob mru %p\n", this->dll.head);
    // printf("prob lru %p\n", this->dll.tail);
    return hitFlag;
  };
  virtual unsigned updateRam(int rS) override {
    unsigned increase = (rS / *this->blockSize) -
                        this->hashMap.size();  // increase in hashmap size
    if (increase > 0) {
      for (int i = 0; i < increase; ++i) {
        this->hashMap.push_back(nullptr);
        protectedLRU.hashMap.push_back(nullptr);
        if ((protectedLRU.hashMap.size() != this->hashMap.size()))
          std::runtime_error("SLRU out of bounds");
      }
    }
    return rS;
  };
};