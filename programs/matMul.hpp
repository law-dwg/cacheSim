#pragma once
#include "../cacheReplacementPolicies/cache.hpp"
#ifndef MATMUL_HPP_
#define MATMUL_HPP_

template <typename R, typename T>
int matMulMain(CacheConfig<R, T> &config, int lhsX, int lhsY, int rhsX,
               int rhsY);

#include "matMul.cpp"

#endif

// template <typename R, typename T>
// struct matMulWrapper {
//   CacheConfig<R,T>& config;
//   matMulWrapper(CacheConfig<R,T>& c, int lhsX, int lhsY, int rhsX, int rhsY)
//   : config(c) {
//     config.program = 0;
//     config.programStr = config.programs[config.program];
//     config.print();
//     int out = matMulMain<R,T>(config, rhsX, rhsY, lhsX, lhsY);
//   };
//   void run(int lhsX, int lhsY, int rhsX, int rhsY) {
//     config.program = 0;
//     config.programStr = config.programs[config.program];
//     config.print();
//     int out = matMulMain<R,T>(config, rhsX, rhsY, lhsX, lhsY);
//     config.writeout();
//   };
// };
template <typename R, typename T>
void matMulWrapper(CacheConfig<R, T> &config, int lhsX, int lhsY, int rhsX,
                   int rhsY)
{
  config.program = 0;
  config.programStr = config.programs[config.program];
  config.print();
  int out = matMulMain<R, T>(config, lhsX, lhsY, rhsX, rhsY);
  // config.writeOut();
};