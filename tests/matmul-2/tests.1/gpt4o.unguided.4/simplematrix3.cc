
#include <stdexcept>
#include "simplematrix.h"

// Define a block size for cache optimization; this should be tuned based on the specific hardware.
constexpr int BLOCK_SIZE = 64;

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  if (lhs.columns() != rhs.rows()) {
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};
  }

  SimpleMatrix res(lhs.rows(), rhs.columns());

  const int lhsRows = lhs.rows();
  const int lhsCols = lhs.columns();
  const int rhsCols = rhs.columns();

  // Blocked matrix multiplication to improve cache usage
  for (int ii = 0; ii < lhsRows; ii += BLOCK_SIZE) {
    for (int jj = 0; jj < rhsCols; jj += BLOCK_SIZE) {
      for (int kk = 0; kk < lhsCols; kk += BLOCK_SIZE) {

        // Process the current block
        for (int i = ii; i < std::min(ii + BLOCK_SIZE, lhsRows); ++i) {
          for (int k = kk; k < std::min(kk + BLOCK_SIZE, lhsCols); ++k) {
            auto lhsValue = lhs(i, k);
            for (int j = jj; j < std::min(jj + BLOCK_SIZE, rhsCols); ++j) {
              res(i, j) += lhsValue * rhs(k, j);
            }
          }
        }
      }
    }
  }

  return res;
}
