#include <stdio.h>

#include <map>
#include <random>
#include <string>
#include <vector>

#include "../../../REF/cacheHitMiss/cache.h"
#include "../cacheReplacementPolicies/cache.hpp"
#include "../utils/wrappers.hpp"
#ifndef MATMUL_HPP_
#define MATMUL_HPP_

template <typename C>
int matMulMain(C &config, int lhsX, int lhsY, int rhsX, int rhsY);

#include "matMul.hpp"

#endif

/*  Assumptions:
      1. All operations performed on one processor (core) with 64KiB (1024 cache
   lines) of L1d cache 2. */
template <typename T>
struct mat {
  unsigned r;
  unsigned c;
  std::vector<T> m;
  mat(unsigned rows, unsigned cols) : r(rows), c(cols) {
    m.resize(rows * cols, 0);
  }
  void print() {
    printf("(%dx%d)=\n[", r, c);
    for (int e = 0; e < r * c; e++) {
      if (e % c == 0) {
        (e == 0) ?: printf("\n ");
      };
      (e == r * c - 1) ? printf("%.3f", m[e]) : printf("%.3f ", m[e]);
    }
    printf("]\n\n");
  }
};

template <typename R, typename T>
mat<T> matMul(CacheConfig<R, T> &config, mat<T> lhs, mat<T> rhs,
              float &hitRatio) {
  int lhsCacheIdx, rhsCacheIdx;
  std::string lhsHitStr, rhsHitStr;
  typedef T data_t;
  typedef R replacementPolicy_t;
  mat<T> out(lhs.r, rhs.c);

  // REFERENCE
  // lap::CacheLRU lru;
  // int ramSize = (lhs.r * lhs.c)+(rhs.r*rhs.c);  // 128
  // int cacheSize = 64;
  // lru.setSize(cacheSize, ramSize);  // baseline
  // END REFERENCE
  CacheWrapper<replacementPolicy_t, data_t, std::vector<data_t>> L1(config);

  auto lhsWrap = L1.allocate(lhs.r * lhs.c, lhs.m);  // datawrappers
  auto rhsWrap = L1.allocate(rhs.r * rhs.c, rhs.m);  // datawrappers

  if (lhs.c == rhs.r) {
    for (int r = 0; r < lhs.r; r++) {
      for (int c = 0; c < rhs.c; c++) {
        float sum = 0;
        for (int i = 0; i < rhs.r; i++) {
          // LHS matrix
          int lhsIdx = r * lhs.c + i;
          int lhsY = lhsIdx - (r * lhs.c);
          int lhsX = (int)(lhsIdx / lhs.c);

          // REFERENCE
          // int cacheline;
          // lhsCacheIdx = lhsIdx;
          // bool lhsHit = lru.find(cacheline, lhsCacheIdx);
          // printf("%d\n",lhsHit);
          // END REFERENCE

          // RHS matrix
          int rhsIdx = c + i * rhs.c;
          int rhsY = rhsIdx - (i * rhs.c);
          int rhsX = (int)(rhsIdx / rhs.c);

          // REFERENCE
          // rhsCacheIdx = (lhs.r * lhs.c) + rhsIdx;
          // bool rhsHit = lru.find(cacheline, rhsCacheIdx);
          // printf("%d\n",rhsHit);
          // END REFERENCE

          sum +=
              lhsWrap[lhsIdx] * rhsWrap[rhsIdx];  // multiple reads, but assume
                                                  // this is in the register
          // printf("");
          // printf("lhs[%d][%d]=%.3f cache %s\nrhs[%d][%d]=%.3f cache %s\n",
          // lhsX, lhsY, lhs.m[lhsIdx], lhsHitStr.c_str(), rhsX, rhsY,
          // rhs.m[rhsIdx],
          //        rhsHitStr.c_str());
          // printf("lhs[%d][%d] cache %s\nrhs[%d][%d] cache %s\n", lhsX, lhsY,
          // lhsHitStr.c_str(), rhsX, rhsY, rhsHitStr.c_str());
          // printf("---------------------------------------------------------------------------------\n");
        }
        out.m[r * out.c + c] = sum;  // written to main memory
      }
    }
    // printf("hitcount = %d, misscount = %d\n", L1.cache.hitCount,
    // L1.cache.missCount); hitRatio = (L1.cache.hitCount /
    // (float)(L1.cache.hitCount + L1.cache.missCount)); std::cout << "hit ratio
    // = " << hitRatio * 100 << "%\n" << std::endl;
  } else {
    printf("Cannot perform multiplication, dimension mismatch %s(%d)\n",
           __FILE__, __LINE__);
    exit(1);
  }
  return out;
};

// Assumptions:
// 1. incremental sums are saved in registers
// 2. cache is empty at beginning
//
template <typename R, typename T>
int matMulMain(CacheConfig<R, T> &config, int lhsX, int lhsY, int rhsX,
               int rhsY) {
  float hitRatio;
  typedef T matType_t;
  mat<matType_t> lhs(lhsX, lhsY);  // too small
  mat<matType_t> rhs(rhsX, rhsY);  // too small
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<std::mt19937::result_type> dist6(
      1, 25);  // distribution in range [1, 6]
  for (int i = 0; i < rhs.r * rhs.c; ++i) {
    rhs.m[i] = dist6(rng);
    // std::cout << &rhs.m[i] << std::endl;
  };
  for (int i = 0; i < lhs.r * lhs.c; ++i) {
    lhs.m[i] = dist6(rng);
    // std::cout << &lhs.m[i] << std::endl;
  };
  printf("lhs mat:\n");
  lhs.print();
  printf("rhs mat:\n");
  rhs.print();
  mat<matType_t> out = matMul<R, T>(config, lhs, rhs, hitRatio);
  // printf("lhs*rhs=:\n");
  // out.print();
  return 0;
};