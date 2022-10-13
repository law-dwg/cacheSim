#pragma once
#include <stdio.h>

#include <iostream>
#include <vector>

// Doubly linked list
template <typename T>
struct Block {
  // int valid = 0;
  // int cacheBlockSize;
  unsigned freq = 0;  // only used for frequency based algs
  // std::vector<T> data;
  Block<T> *prev = nullptr;  // more recent
  Block<T> *next = nullptr;  // less recent
  Block<T> **tag = nullptr;  // location in main memory (POINTS TO HASHMAP)
  // Node<T>() { std::cout << "node is generated" << std::endl; }
  //~Node<T>() { std::cout << "node is destroyed" << std::endl; }
  void reset() {
    // data.clear();
    prev = nullptr;
    next = nullptr;
    if (tag != nullptr) *tag = nullptr;  // update hashmap
    tag = nullptr;
    freq = 0;
  };
};

template <typename T>
struct DoublyLinkedList {
  // data members
  Block<T> *head = nullptr;
  Block<T> *tail = nullptr;
  int counter = 0;
  bool full = false;
  const int size;  // number of blocks in dll
  std::vector<Block<T>>
      blocks;  // abstraction of the cache in blocks sizeOf(blocks)=size

  /** Constructor **/
  DoublyLinkedList<T>(unsigned blockSize, unsigned size)
      : blocks(size),
        size(size){
            // for (auto b = blocks.begin(); b != blocks.end(); ++b) {
            //   b->data.resize(blockSize);
            // };
        };

  // function members
  bool isHead(Block<T> *blkPtr) { return head == blkPtr ? true : false; };
  bool isTail(Block<T> *blkPtr) { return tail == blkPtr ? true : false; };
  // void insertAfter(Block<T>* blk, Block<T>* newblk) {  // insert in direction
  // tail
  //   // head  <-head->.....<-tail-> tail
  //   newblk->prev = blk;
  //   if (blk->next == nullptr) {  // tail
  //     newblk->next = nullptr;
  //     tail = newblk;
  //   } else {
  //     newblk->next = blk->next;
  //     blk->next->prev = newblk;
  //   }
  //   blk->next = newblk;
  // };
  // void insertEnd(Block<T>* newblk) {  // insert at tail
  //   if (tail == nullptr) {
  //     insertBeginning(newblk);
  //   } else {
  //     if (tail != newblk) {
  //       insertAfter(tail, newblk);
  //     };
  //   }
  // };
  void insertBefore(Block<T> *blk,
                    Block<T> *newblk) {  // insert in direction head
    // head  <-head->.....<-tail-> tail
    // (before)<-prev.....next->(after)
    newblk->next = blk;
    if (blk == head) {  // head
      newblk->prev = nullptr;
      head = newblk;
    } else {
      newblk->prev = blk->prev;
      blk->prev->next = newblk;
    }
    blk->prev = newblk;
  };
  void insertBeginning(Block<T> *newblk) {  // insert at head
    if (head == nullptr) {
      head = newblk;
      tail = newblk;
      newblk->prev = nullptr;
      newblk->next = nullptr;
    } else {
      insertBefore(head, newblk);
    }
  };
  void remove(Block<T> *blk) {
    if (head == blk) {  // if node is head
      head = blk->next;
    } else {
      blk->prev->next = blk->next;
    }

    if (tail == blk) {  // if node is tail
      tail = blk->prev;
    } else {
      blk->next->prev = blk->prev;
    }
  }
};

template <typename T>
class Set {  // abstract class
 public:
  // data members
  int *const blockSize;     // # of enries in block stored on cache level
  DoublyLinkedList<T> dll;  // doubly linked list of cache blocks
  std::vector<Block<T> *>
      hashMap;  // points to the cache (essentially a copy of the main memory)

  /** constructor **/
  Set(int sS, int *bS, int hMS)
      : blockSize(bS), dll(*bS, sS), hashMap(hMS, nullptr){};

  // function members
  bool hit(Block<T> *hashMapEntry) {
    return hashMapEntry != nullptr ? true : false;
  };
  void updateNewBlock(int offset, int tag, T &ramData,
                      Block<T> *blkPtr) {  // relevant update for all sets
    // hashmap updates
    hashMap[tag] = blkPtr;  // update hashmap
    // block updates
    blkPtr->tag = &hashMap[tag];  // pointing to locating in main mem
    // T* ramPtrBeg = &ramData - offset;
    // T* ramPtrEnd = (&ramData - offset) + *blockSize;
    // blkPtr->data.assign(ramPtrBeg, ramPtrEnd);
  };
  void increment() {
    ++dll.counter;
    (dll.counter < dll.size) ? dll.full = false : dll.full = true;
  };
  void decrement() {
    --dll.counter;
    dll.full = false;
  };
  virtual void reset() {
    dll.blocks.clear();
    dll.blocks.resize(dll.size);
    std::fill(hashMap.begin(), hashMap.end(), nullptr);
    dll.counter = 0;
    dll.full = false;
  };

  // pure virtuals
  virtual bool find(int offset, int tag, T &ramData) = 0;
  virtual std::string name() = 0;

  // virtuals
  virtual unsigned updateRam(int rS) {
    unsigned increase = (((rS - 1) / *blockSize) + 1) -
                        hashMap.size();  // increase in hashmap size
    if (increase > 0) {
      for (int i = 0; i < increase; ++i) {
        hashMap.push_back(nullptr);
      }
    }
    return rS;
  };
};