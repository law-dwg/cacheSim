#include <stdio.h>

#include <cstdio>
#include <thread>

#include "cache/cache.hpp"
#include "programs/matMul.hpp"
#include "programs/sort.hpp"

namespace t {
// types
typedef double dataType_t;
typedef FIFO<dataType_t> FIFO_t;
typedef LFU<dataType_t> LFU_t;
typedef LRU<dataType_t> LRU_t;
typedef MRU<dataType_t> MRU_t;
typedef PLRU<dataType_t> PLRU_t;
typedef SLRU<dataType_t> SLRU_t;
}  // namespace t

template <typename R, typename T>
CacheConfig<R, T> createConfig(int bS, int cS, int a) {
  return CacheConfig<R, T>(bS, cS, a);
};

void matMulExe(CacheConfig<t::FIFO_t, t::dataType_t> &cc_fifo,  // CC_FIFO
               CacheConfig<t::LFU_t, t::dataType_t> &cc_lfu,    // CC_LFU
               // CacheConfig<t::LRU_t, t::dataType_t> &cc_lru,   // CC_LRU
               CacheConfig<t::MRU_t, t::dataType_t> &cc_mru,    // CC_MRU
               CacheConfig<t::PLRU_t, t::dataType_t> &cc_plru,  // CC_PLRU
               CacheConfig<t::SLRU_t, t::dataType_t> &cc_slru,  // CC_SLRU
               int rangeLow,                                    // rL
               int rangeHigh,                                   // rH
               std::mt19937 &mt                                 // mt
) {
  int lhsX, lhsY, rhsX, rhsY;
  std::uniform_int_distribution<> dist1(1, 5);
  std::uniform_int_distribution<> dist2(14, 20);
  std::uniform_int_distribution<> dist3(1, 5);
  lhsX = dist2(mt);
  lhsY = dist1(mt);
  rhsY = dist2(mt);
  rhsX = lhsY;  // ensure proper dimensioning
  matMulWrapper<t::FIFO_t, t::dataType_t>(cc_fifo, lhsX, lhsY, rhsX, rhsY);
  matMulWrapper<t::LFU_t, t::dataType_t>(cc_lfu, lhsX, lhsY, rhsX, rhsY);
  // matMulWrapper<t::LRU_t, t::dataType_t>(cc_lru, lhsX, lhsY, rhsX, rhsY);
  matMulWrapper<t::MRU_t, t::dataType_t>(cc_mru, lhsX, lhsY, rhsX, rhsY);
  matMulWrapper<t::PLRU_t, t::dataType_t>(cc_plru, lhsX, lhsY, rhsX, rhsY);
  matMulWrapper<t::SLRU_t, t::dataType_t>(cc_slru, lhsX, lhsY, rhsX, rhsY);
};

void sortExe(CacheConfig<t::FIFO_t, t::dataType_t> &cc_fifo,  // CC_FIFO
             CacheConfig<t::LFU_t, t::dataType_t> &cc_lfu,    // CC_LFU
             // CacheConfig<t::LRU_t, t::dataType_t> &cc_lru,   // CC_LRU
             CacheConfig<t::MRU_t, t::dataType_t> &cc_mru,    // CC_MRU
             CacheConfig<t::PLRU_t, t::dataType_t> &cc_plru,  // CC_PLRU
             CacheConfig<t::SLRU_t, t::dataType_t> &cc_slru,  // CC_SLRU
             int rangeLow,                                    // rL
             int rangeHigh,                                   // rH
             std::mt19937 &mt                                 // mt
) {
  int vectDim;
  std::uniform_int_distribution<> dist(rangeLow, rangeHigh);
  vectDim = dist(mt);
  writeArray<t::dataType_t>(0, 1000, vectDim, TEMP_SORT_FILE);
  sortWrapper<t::FIFO_t, t::dataType_t>(cc_fifo, vectDim);
  sortWrapper<t::LFU_t, t::dataType_t>(cc_lfu, vectDim);
  // sortWrapper<t::LRU_t, t::dataType_t>(cc_lru, vectDim);
  sortWrapper<t::MRU_t, t::dataType_t>(cc_mru, vectDim);
  sortWrapper<t::PLRU_t, t::dataType_t>(cc_plru, vectDim);
  sortWrapper<t::SLRU_t, t::dataType_t>(cc_slru, vectDim);
  deleteArray(TEMP_SORT_FILE);
};

int main() {
  initGlobalConfig();

  //// matMul Specs
  int entrySize = sizeof(t::dataType_t);  // size of entry
  int blockSize = entrySize;              // blockSize in Bytes
  int cacheSize = blockSize * 16 * 2;     // cacheSize in Bytes
  // int a = 8;
  int m_matrix = 14;  // lower bound;
  int n_matrix = 18;  // uppder bound;

  // sort Specs
  // int entrySize = sizeof(t::dataType_t); // size of entry
  // int blockSize = entrySize;             // blockSize in Bytes
  // int cacheSize = blockSize * 8;         // cacheSize in Bytes
  // // int a = 2;
  // int m_sort = 29;  // lower bound; 5
  // int n_sort = 34; // uppder bound; 16
  std::vector<int> a = {4};
  std::random_device rd;
  std::mt19937 mt(rd());

  int numofiter = 16 * 2;  // number of sequences
  for (int q = 0; q < a.size(); ++q) {
    int counter = 0;
    while (counter < numofiter) {
      CacheConfig<t::FIFO_t, t::dataType_t> CC_FIFO(blockSize, cacheSize, a[q]);
      CacheConfig<t::LFU_t, t::dataType_t> CC_LFU(blockSize, cacheSize, a[q]);
      CacheConfig<t::LRU_t, t::dataType_t> CC_LRU(blockSize, cacheSize, a[q]);
      CacheConfig<t::MRU_t, t::dataType_t> CC_MRU(blockSize, cacheSize, a[q]);
      CacheConfig<t::PLRU_t, t::dataType_t> CC_PLRU(blockSize, cacheSize, a[q]);
      CacheConfig<t::SLRU_t, t::dataType_t> CC_SLRU(blockSize, cacheSize, a[q]);
      std::vector<double> perm;
      matMulExe(CC_FIFO,  //
                CC_LFU,   //
                // CC_LRU,  //
                CC_MRU,    //
                CC_PLRU,   //
                CC_SLRU,   //
                m_matrix,  //
                n_matrix,  //
                mt         //
      );

      // sortExe(CC_FIFO, //
      //         CC_LFU,  //
      //         // CC_LRU,  //
      //         CC_MRU,  //
      //         CC_PLRU, //
      //         CC_SLRU, //
      //         m_sort,  //
      //         n_sort,  //
      //         mt       //
      // );

      perm.push_back((double)CC_FIFO.hitCount /
                     (double)(CC_FIFO.hitCount + CC_FIFO.missCount));
      perm.push_back((double)CC_LFU.hitCount /
                     (double)(CC_LFU.hitCount + CC_LFU.missCount));
      // perm.push_back((double)CC_LRU.hitCount / (double)(CC_LRU.hitCount +
      // CC_LRU.missCount));
      perm.push_back((double)CC_MRU.hitCount /
                     (double)(CC_MRU.hitCount + CC_MRU.missCount));
      perm.push_back((double)CC_PLRU.hitCount /
                     (double)(CC_PLRU.hitCount + CC_PLRU.missCount));
      perm.push_back((double)CC_SLRU.hitCount /
                     (double)(CC_SLRU.hitCount + CC_SLRU.missCount));

      for (int i = 0; i < perm.size(); ++i) {
        std::cout << perm[i] << std::endl;
      };

      int temp = 0;
      std::sort(perm.begin(), perm.end());
      int e = 0;
      do {
        if (perm[0] == perm[1]) {
          e = 1;
        } else {
          e = 0;
        };
        ++temp;
      } while (std::next_permutation(perm.begin(), perm.end()) && e == 0);
      std::cout << temp << " of permutations" << std::endl;

      if (e) {
        std::cout << "There are two that are likely identical" << std::endl;
        std::cout << "I will remove these files for you and try another set of "
                     "matricies"
                  << std::endl;
        std::remove(CC_FIFO.fileoutData.c_str());
        std::remove(CC_LFU.fileoutData.c_str());
        // std::remove(CC_LRU.fileoutData.c_str());
        std::remove(CC_MRU.fileoutData.c_str());
        std::remove(CC_PLRU.fileoutData.c_str());
        std::remove(CC_SLRU.fileoutData.c_str());
        printf(
            "%d/%d "
            "retry\n--------------------------------------------------------"
            "----------\n",
            counter + 1, numofiter);
      } else {
        CC_FIFO.writeOut(counter);
        CC_LFU.writeOut(counter);
        // CC_LRU.writeOut(counter);
        CC_MRU.writeOut(counter);
        CC_PLRU.writeOut(counter);
        CC_SLRU.writeOut(counter);
        printf(
            "%d/%d "
            "completed\n----------------------------------------------------"
            "--------------\n",
            counter + 1, numofiter);
        ++counter;
      };
      std::this_thread::sleep_for(
          std::chrono::milliseconds(1));  // make sure not overwritten
    };
  }
  return 0;
};