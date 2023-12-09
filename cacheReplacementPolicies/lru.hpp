#pragma once
#include "set.hpp"

template <typename T>
class LRU : public Set<T>
{
public:
  /** LRU Cache Replacement Policy
   * LRU is the next block to be evicted (tail)
   * MRU is the last to be evicted (head) **/

  // data members
  Block<T> lastEvicted; // cacheline of last evicted

  /** constructor **/
  LRU(int sS, int *bS, int hMS) : Set<T>(sS, bS, hMS){};

  // function members
  std::string name() { return "LRU"; };

  void hitLRU(int tag, Block<T> *blkPtr)
  {
    this->dll.remove(blkPtr);
    this->dll.insertBeginning(blkPtr);
  };

  void missLRU(int offset, int tag, T &ramData, Block<T> *blkPtr)
  {
    if (!this->dll.full)
    {                                                // if cache not full
      blkPtr = &this->dll.blocks[this->dll.counter]; // new node
      this->increment();                             // update counter
    }
    else
    {                          // if cache full
      blkPtr = this->dll.tail; // LRU
      lastEvicted = *blkPtr;
      this->dll.remove(blkPtr);
      blkPtr->reset();
    }
    this->updateNewBlock(offset, tag, ramData, blkPtr);
    this->dll.insertBeginning(blkPtr);
  };

  bool find(int offset, int tag, T &ramData)
  { // processor requests memory
    bool hitFlag;
    Block<T> *blkPtr;
    // std::cout<<"BEFORE"<<std::endl;
    // for (int i = 0; i < this->hashMap.size(); ++i) {
    //   if (this->hashMap[i] != nullptr) {
    //     printf("%p <- address %i %p -> %p\n", this->hashMap[i]->prev,  i,
    //     this->hashMap[i], this->hashMap[i]->next);
    //   }
    // };
    if (this->hit(this->hashMap[tag]))
    {                 // CACHE HIT
      hitFlag = true; // hit
      if (this->dll.size != 1)
      {                              // if direct mapping, you dont need to move
                                     // anything, it stays where it is
        blkPtr = this->hashMap[tag]; // set the working pointer
        hitLRU(tag, blkPtr);
      }
    }
    else
    { // CACHE MISS
      hitFlag = false;
      missLRU(offset, tag, ramData, blkPtr);
    }

    // DEBUGGING
    // std::cout<<"AFTER"<<std::endl;
    // printf("HMS = %d\n",this->hashMap.size());
    // for (int i = 0; i < this->hashMap.size(); ++i) {
    //   if (this->hashMap[i] != nullptr) {
    //     printf("%p <- address %i %p -> %p\n", this->hashMap[i]->prev,  i,
    //     this->hashMap[i], this->hashMap[i]->next);
    //   }
    // };
    // printf("h/m: %d\n",hitFlag);
    // printf("markedsize %d\n", this->dll.size);
    // printf("counter %i full = %d\n", this->dll.counter, this->dll.full);
    // printf("mru %p\n", this->dll.head);
    // printf("lru %p\n", this->dll.tail);
    // printf("\n");

    return hitFlag;
  };
};