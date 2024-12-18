
#include <stdexcept>
#include <algorithm> // for std::min
#include "simplematrix.h"

constexpr int BLOCK_SIZE = 64;

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  if (lhs.columns() != rhs.rows()) {
      throw std::runtime_error{"lhs.columns() != rhs.rows()"};
  }

  SimpleMatrix res(lhs.rows(), rhs.columns());

  const int lhsRows = lhs.rows();
  const int lhsCols = lhs.columns();
  const int rhsCols = rhs.columns();

  // Blocked matrix multiplication with further optimizations
  for (int ii = 0; ii < lhsRows; ii += BLOCK_SIZE) {
    for (int jj = 0; jj < rhsCols; jj += BLOCK_SIZE) {
      for (int kk = 0; kk < lhsCols; kk += BLOCK_SIZE) {

        for (int i = ii; i < std::min(ii + BLOCK_SIZE, lhsRows); ++i) {
          for (int k = kk; k < std::min(kk + BLOCK_SIZE, lhsCols); ++k) {
              auto lhsValue = lhs(i, k);

              // Aggressively unroll the inner loop to encourage SIMD vectorization
              int j = jj;
              for (; j <= std::min(jj + BLOCK_SIZE, rhsCols) - 4; j += 4) {
                res(i, j)     += lhsValue * rhs(k, j);
                res(i, j + 1) += lhsValue * rhs(k, j + 1);
                res(i, j + 2) += lhsValue * rhs(k, j + 2);
                res(i, j + 3) += lhsValue * rhs(k, j + 3);
              }

              // Handle remaining elements (if any)
              for (; j < std::min(jj + BLOCK_SIZE, rhsCols); ++j) {
                res(i, j) += lhsValue * rhs(k, j);
              }
          }
        }
      }
    }
  }

  return res;
}
