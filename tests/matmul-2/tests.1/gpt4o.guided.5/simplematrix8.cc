
#include <algorithm>
#include <stdexcept>
#include "simplematrix.h"

// Define block size for cache utilization, consider tuning this based on profiling
const int BLOCK_SIZE = 64;  // Adjusting block size for potentially better performance

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  if (lhs.columns() != rhs.rows()) {
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};
  }

  SimpleMatrix res(lhs.rows(), rhs.columns());

  const int lhsRows = lhs.rows();
  const int rhsColumns = rhs.columns();
  const int sharedDim = lhs.columns();

  // Initialize intermediate buffer to minimize access costs
  for (int ii = 0; ii < lhsRows; ii += BLOCK_SIZE) {
    for (int jj = 0; jj < rhsColumns; jj += BLOCK_SIZE) {
      for (int kk = 0; kk < sharedDim; kk += BLOCK_SIZE) {
        
        for (int i = ii; i < std::min(ii + BLOCK_SIZE, lhsRows); ++i) {
          for (int j = jj; j < std::min(jj + BLOCK_SIZE, rhsColumns); ++j) {
            SimpleMatrix::value_type sum = res(i, j);

            for (int k = kk; k < std::min(kk + BLOCK_SIZE, sharedDim); ++k) {
              sum += lhs(i, k) * rhs(k, j);
            }

            res(i, j) = sum;
          }
        }
      }
    }
  }

  return res;
}
