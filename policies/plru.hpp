#pragma once
#include "../cache/set.hpp"

template <typename T>
struct node
{
  node *left;
  node *right;
  node *above;
  bool pointsRight; // 0(false) = left, 1(true) = right
  node(node *l, node *r) : left(l), right(r), pointsRight(false){};
  void switchNodes() { pointsRight = !pointsRight; };
  virtual Block<T> *descend()
  { // returns ptr of replaced block
    Block<T> *blkPtr = (pointsRight) ? right->descend() : left->descend();
    switchNodes();
    return blkPtr;
  };
  void ascend(node *belowPtr,
              node *selfPtr)
  { // switches ptr if its being ptd to
    if ((pointsRight && belowPtr == right) ||
        (!pointsRight && belowPtr == left))
    {
      switchNodes();
    };
    if (above != nullptr)
    {
      above->ascend(selfPtr, above);
    };
  };
};

template <typename T>
struct endNode : public node<T>
{
private:
  using node<T>::ascend; // blocked from using
public:
  Block<T> *leftEnd;
  Block<T> *rightEnd;
  // node* left = nullptr;
  // node* right = nullptr;
  endNode(Block<T> *l, Block<T> *r)
      : node<T>(nullptr, nullptr),
        leftEnd(l),
        rightEnd(r){
            // printf("ptrleft = %p, ptrright= %p\n",l,r);
        };
  Block<T> *descend() override
  { // returns ptr to proper block
    Block<T> *blkPtr = (this->pointsRight) ? rightEnd : leftEnd;
    this->switchNodes();
    return blkPtr;
  };

  void ascendEnd(Block<T> *belowPtr,
                 node<T> *selfPtr)
  { // switches ptr if its being ptd to
    if ((this->pointsRight && belowPtr == rightEnd) ||
        (!this->pointsRight && belowPtr == leftEnd))
    {
      this->switchNodes(); // point away from block if hit
    };
    if (this->above != nullptr)
    {
      this->above->ascend(selfPtr, this->above);
    }
  };
};

template <typename T>
class PLRU : public Set<T>
{
public:
  // data members
  Block<T> lastEvicted;                        // cacheline of last evicted
  std::vector<std::shared_ptr<node<T>>> nodes; // size = setSize-1
  /** constructor **/
  PLRU(int sS, int *bS, int hMS) : Set<T>(sS, bS, hMS)
  {
    int sq = std::log2(sS); // ensure its a whole number
    double cond = pow(2, sq);
    if (cond != sS)
      throw std::runtime_error("set size must be square e.g. 2,4,8,16,32,64\n");
    int n_nodes = sS - 1;    // number of nodes in tree
    int n_endNodes = sS / 2; // number of end nodes in tree
    if (sS > 2)
    {
      for (int i = sS; i > 0; i -= 2)
      { // end nodes
        int idxleft = i - 2;
        int idxright = i - 1;
        Block<T> *ptrleft = &this->dll.blocks[idxleft];
        Block<T> *ptrright = &this->dll.blocks[idxright];
        // printf("idxleft = %d, idxright=%d, ptrleft = %p, ptrright=
        // %p\n",idxleft,idxright,ptrleft,ptrright);
        nodes.emplace(nodes.begin(),
                      std::make_shared<endNode<T>>(ptrleft, ptrright));
      };
      if (sS > 4)
      {
        int counter = 1;
        for (int i = (n_nodes - n_endNodes - 1); i > 0; --i)
        { // mid nodes
          int gapRight = n_endNodes - counter;
          int gapLeft = gapRight - 1;
          node<T> *nodePtrLeft = nodes[gapLeft].get();
          node<T> *nodePtrRight = nodes[gapRight].get();
          nodes.emplace(nodes.begin(),
                        std::make_shared<node<T>>(nodePtrLeft, nodePtrRight));
          ++counter;
        };
      };
      nodes.emplace(nodes.begin(),
                    std::make_shared<node<T>>(nodes[0].get(),
                                              nodes[1].get())); // root node
    }
    else
    { // only two blocks
      Block<T> *ptrleft = &this->dll.blocks[0];
      Block<T> *ptrright = &this->dll.blocks[1];
      nodes.emplace(nodes.begin(),
                    std::make_shared<endNode<T>>(ptrleft, ptrright));
    };
    nodes[0]->above = nullptr; // nothing is above root node
    for (int i = (n_nodes - 1); i > 0; --i)
    {                         // above nodes
      int prev = (i - 1) / 2; // idx of prev node
      nodes[i]->above = nodes[prev].get();
    };
    // DEBUGGING
    // for (int i = 0; i < nodes.size(); ++i) {
    //   printf("%d node @ %p: left=%p right =%p | above =%p | ", i,
    //   this->nodes[i].get(), this->nodes[i]->left, this->nodes[i]->right,
    //   this->nodes[i]->above); if(i >= n_endNodes-1){
    //     endNode<T>* eN = dynamic_cast<endNode<T>*>(this->nodes[i].get());
    //     printf("leftEnd = %p rightEnd = %p ",eN->leftEnd, eN->rightEnd);
    //   }
    //   printf("\n");
    // };
  };

  // function members
  std::string name() { return "PLRU"; };

  void hitPLRU(int tag, Block<T> *blkPtr)
  {
    unsigned blkIdx = (unsigned)(this->hashMap[tag] - &this->dll.blocks[0]);
    unsigned nodeIdx = blkIdx / 2 + ((this->dll.size / 2) - 1);
    blkPtr = &this->dll.blocks[blkIdx];
    endNode<T> *eN = dynamic_cast<endNode<T> *>(nodes[nodeIdx].get());
    eN->ascendEnd(blkPtr, nodes[nodeIdx].get()); // working upwards
    // pointers now point away from this block
  };

  void missPLRU(int offset, int tag, T &ramData, Block<T> *blkPtr)
  {
    // start from root node and descend
    // during descent the switches are updated
    blkPtr = nodes[0]->descend(); // this is the new block where data must go
    blkPtr->reset();              // clear it and update hashmap
    this->updateNewBlock(offset, tag, ramData, blkPtr);
  };

  bool find(int offset, int tag, T &ramData)
  { // processor requests memory
    bool hitFlag;
    Block<T> *blkPtr;
    if (this->hit(this->hashMap[tag]))
    {                 // CACHE HIT
      hitFlag = true; // hit
      if (this->dll.size != 1)
      {                              // if direct mapping, you dont need to move
                                     // anything, it stays where it is
        blkPtr = this->hashMap[tag]; // set the working pointer
        hitPLRU(tag, blkPtr);
      }
    }
    else
    { // CACHE MISS
      hitFlag = false;
      missPLRU(offset, tag, ramData, blkPtr);
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