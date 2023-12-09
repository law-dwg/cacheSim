#pragma once
#include <map>
#include <memory>

#include "lru.hpp"

// you should probably rebuild your LRU containers for the operations used
// within the FreqMap
template <typename T>
class basicLRU
{
public:
  // data members
  Block<T> *mru = nullptr;
  Block<T> *lru = nullptr;
  std::shared_ptr<basicLRU> moreFreq = nullptr;
  std::shared_ptr<basicLRU> lessFreq = nullptr;
  int size = 0;

  /** constructor **/
  // basicLRU(){};

  // function members
  void reset()
  {
    mru = nullptr;
    lru = nullptr;
    moreFreq = nullptr;
    lessFreq = nullptr;
    size = 0;
  };

  void set(Block<T> *blkPtr)
  {
    if (size == 0)
    {
      mru = blkPtr;
      lru = blkPtr;
      blkPtr->prev = nullptr;
      blkPtr->next = nullptr;
    }
    else if (size > 0)
    {
      mru->prev = blkPtr;     // blkPtr <- oldMRU ...
      blkPtr->next = mru;     // blkPtr -> oldMRU ...
      blkPtr->prev = nullptr; // null <- blkPtr -> <- oldMRU ...
      mru = blkPtr;           // null <- newMRU/blkPtr -> <- oldMRU ...
    }
    else
    {
      throw std::runtime_error("theres a problem with your basicLRU insert");
    }
    size++;
  };

  void remove(Block<T> *blkPtr)
  {
    if (size == 1)
    { // only one element left
      lru = nullptr;
      mru = nullptr;
    }
    else if (size > 1)
    {
      if (blkPtr == lru)
      {                            // if lru
        lru->prev->next = nullptr; // ... newLRU -> nullptr |  newLRU <-
                                   // oldLRU/blkPtr -> nullptr
        lru = lru->prev;           // new lru
        blkPtr->prev = nullptr;    // ... newLRU -> nullptr |  nullptr <-
                                   // oldLRU/blkPtr -> nullptr
      }
      else if (blkPtr == mru)
      {                            // if mru
        mru->next->prev = nullptr; // nullptr <- oldMRU/blkPtr -> newMRU |
                                   // nullptr <- newMRU ...
        mru = mru->next;           // new mru
        blkPtr->next =
            nullptr; // nullptr <- oldMRU/blkPtr -> nullptr | nullptr
                     // <- newMRU ...
      }
      else
      { // if in middle (swap)
        blkPtr->prev->next = blkPtr->next;
        blkPtr->next->prev = blkPtr->prev;
      }
    }
    else
    {
      throw std::runtime_error("theres a problem with your basicLRU remove");
    }
    // disconnect from this LRU
    blkPtr->prev = nullptr;
    blkPtr->next = nullptr;
    size--;
  }
};

template <typename T>
class LFU : public Set<T>
{
public:
  /** LFU
   * Map is stores the frequencies and the corresponding blocks that have those
   * frequencies with their own replacement algorithmn (e.g. FIFO or LRU)
   */

  // data members
  std::map<unsigned, std::shared_ptr<basicLRU<T>>>
      freqMap; // max size == setSize
  std::shared_ptr<basicLRU<T>> mfu = nullptr;
  std::shared_ptr<basicLRU<T>> lfu = nullptr;

  /** constructor **/
  LFU(int sS, int *bS, int hMS) : Set<T>(sS, bS, hMS)
  {
    if (sS == 1)
      throw std::runtime_error(
          "LFU is not supported for direct mapping "
          "(doesnt make sense to implement)\n");
    std::shared_ptr<basicLRU<T>> blruPtr(new basicLRU<T>);
    freqMap.insert({0, blruPtr}); // starting out
    lfu = blruPtr;
    mfu = blruPtr;
  };

  // function members
  std::string name() { return "LFU"; };
  bool isKeyInMap(unsigned key)
  {
    if (freqMap.find(key) == freqMap.end())
    {
      return false;
    }
    else
    {
      return true;
    }
  };

  void insertAfter(std::shared_ptr<basicLRU<T>> bLRU,
                   std::shared_ptr<basicLRU<T>> newbLRU)
  { // direction lfu
    // mfu  <-head->.....<-tail-> lfu
    newbLRU->moreFreq = bLRU;
    if (bLRU->lessFreq == nullptr)
    { // LFU
      newbLRU->lessFreq = nullptr;
      lfu = newbLRU;
    }
    else
    {
      newbLRU->lessFreq = bLRU->lessFreq;
      bLRU->lessFreq->moreFreq = newbLRU;
    }
    bLRU->lessFreq = newbLRU;
  };

  void insertBefore(std::shared_ptr<basicLRU<T>> bLRU,
                    std::shared_ptr<basicLRU<T>> newbLRU)
  { // direction mfu
    // mfu  <-head->.....<-tail-> lfu
    // (before)<-moreFreq.....lessFreq->(after)
    newbLRU->lessFreq = bLRU;
    if (bLRU->moreFreq == nullptr)
    { // MFU
      newbLRU->moreFreq = nullptr;
      mfu = newbLRU;
    }
    else
    {
      newbLRU->moreFreq = bLRU->moreFreq;
      bLRU->moreFreq->lessFreq = newbLRU;
    }
    bLRU->moreFreq = newbLRU;
  };

  void insertMFU(std::shared_ptr<basicLRU<T>> newbLRU)
  {
    // insertBeginning
    //// if we didnt initialize our list we could use commented out code
    // if (mfu == nullptr){
    //   mfu = newbLRU;
    //   lfu = newbLRU;
    //   newbLRU->moreFreq = nullptr;
    //   newbLRU->lessFreq = nullptr;
    // } else{
    insertBefore(mfu, newbLRU);
    // }
  };

  void insertLFU(std::shared_ptr<basicLRU<T>> newbLRU)
  { // insertEnd
    // if we didnt initialize our list we could use commented out code
    // if(lfu == nullptr){
    //   insertMFU(newbLRU);
    // } else {
    insertAfter(lfu, newbLRU);
    // }
  };

  void remove(std::shared_ptr<basicLRU<T>> bLRU)
  {
    if (bLRU->moreFreq == nullptr)
    { // if node is MFU
      mfu = bLRU->lessFreq;
    }
    else
    {
      bLRU->moreFreq->lessFreq = bLRU->lessFreq;
    }

    if (bLRU->lessFreq == nullptr)
    { // if node is LFU
      lfu = bLRU->moreFreq;
    }
    else
    {
      bLRU->lessFreq->moreFreq = bLRU->moreFreq;
    }
    // node can be reset
    bLRU->reset();
  }

  void missLFU(int offset, int tag, T &ramData, Block<T> *blkPtr)
  {
    // 0 freq group is never removed!
    unsigned oldFreq;
    // map scope
    if (this->dll.full)
    {                         // recycling an existing cache-block
      blkPtr = lfu->lru;      // still contains old data
      oldFreq = blkPtr->freq; // keep useful data
      lfu->remove(blkPtr);    // block floating unassigned
      blkPtr->reset();        // block scope - clear old data, updates hashmap as well
                              // (you should change this)
      if (freqMap[oldFreq]->size == 0)
      {
        remove(freqMap[oldFreq]); // remove old if necessary
        if (oldFreq != 0)
        { // we going to let 0 group float
          // delete freqMap[oldFreq];                                  //
          // dangling ptr
          freqMap[oldFreq] = nullptr; // block dangling ptr
          freqMap.erase(oldFreq);
        }
      }
    }
    else
    {                                                // pulling a fresh unused cache-block
      blkPtr = &this->dll.blocks[this->dll.counter]; // new node
      this->increment();                             // next available node index
    }
    // block scope
    this->updateNewBlock(offset, tag, ramData,
                         blkPtr); // block updated with new info
    // map scope
    if (lfu != freqMap[0])
      insertAfter(lfu, freqMap[0]); // new lfu is set if necessary
    freqMap[0]->set(blkPtr);
  };

  void hitLFU(int offset, int tag, T &ramData, Block<T> *blkPtr)
  {
    // block scope
    unsigned oldFreq = blkPtr->freq;
    blkPtr->freq++;
    unsigned newFreq = blkPtr->freq;

    // block/map scope
    freqMap[oldFreq]->remove(blkPtr); // remove block from old map entry

    // we will now move block from oldFreq location to newFreq location

    // map scope
    if (!isKeyInMap(newFreq))
    {                                                     // if new freqGroup doesnt exist
      std::shared_ptr<basicLRU<T>> blru(new basicLRU<T>); // create it
      freqMap.insert(
          {newFreq, blru}); // add to map freqMap[newFreq] now exists
      insertBefore(
          freqMap[oldFreq],
          freqMap[newFreq]); // insert freqMap[newFreq] before existing
    }
    // our new node is already in the list, now check if the old one should be
    // removed we can remove the 0 group, but we wont "hard" delete it.
    if (freqMap[oldFreq]->size == 0)
    {                           // if the old one is now empty
      remove(freqMap[oldFreq]); // floating in space
      if (oldFreq != 0)
      { // we going to let 0 group float
        // delete freqMap[oldFreq];        // dangling ptr
        freqMap[oldFreq] = nullptr; // block dangling ptr
        freqMap.erase(oldFreq);
      }
    }
    // then we set new block
    freqMap[newFreq]->set(blkPtr);
    // end
  };

  bool find(int offset, int tag, T &ramData)
  { // processor requests memory
    bool hitFlag;
    Block<T> *blkPtr;
    if (this->hit(this->hashMap[tag]))
    {                 // CACHE HIT
      hitFlag = true; // hit
      if (this->dll.size != 1)
      {                              // if not direct mapping
        blkPtr = this->hashMap[tag]; // set the working pointer
        hitLFU(offset, tag, ramData, blkPtr);
      }
    }
    else
    { // CACHE MISS
      hitFlag = false;
      missLFU(offset, tag, ramData, blkPtr);
    }

    //   std::string txt = (hitFlag) ? "hit" : "miss";
    //   std::cout << txt << std::endl;
    //   printf("data[%d]=%f\n", offset, this->mfu->mru->data[offset]);
    //   for (int i = 0; i < this->hashMap.size(); ++i) {
    //     if (this->hashMap[i] != nullptr) {
    //       printf("ram address %i %p with freq = %i\n", i, this->hashMap[i],
    //       this->hashMap[i]->freq);
    //     }
    //   };
    //   printf("freqMap:\n");
    //   for (auto const& [key, val] : freqMap) {
    //     std::cout << key         // string (key)
    //               << ':' << val  // string's value
    //               << " with size " << freqMap[key]->size << std::endl;
    //   }
    //   printf("counter %i probfull = %d\n", this->dll.counter,
    //   this->dll.full); printf("mfu %p\n", this->mfu); printf("lfu %p\n",
    //   this->lfu); printf("----------------------------------\n");
    return hitFlag;
  }
};