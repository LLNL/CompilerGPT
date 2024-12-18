
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  if (lhs.columns() != rhs.rows()) {
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};
  }

  using ValueType = SimpleMatrix::value_type;
  const int BLOCK_SIZE = 64; // Block size chosen based on cache line size & trial

  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();

  SimpleMatrix res{lhsRows, rhsCols};

  for (int i = 0; i < lhsRows; i += BLOCK_SIZE) {
    for (int j = 0; j < rhsCols; j += BLOCK_SIZE) {
      for (int k = 0; k < lhsCols; k += BLOCK_SIZE) {
        // Process block sub-matrices
        for (int ii = i; ii < std::min(i + BLOCK_SIZE, lhsRows); ++ii) {
          for (int jj = j; jj < std::min(j + BLOCK_SIZE, rhsCols); ++jj) {
            ValueType sum = res(ii, jj);
            for (int kk = k; kk < std::min(k + BLOCK_SIZE, lhsCols); ++kk) {
              sum += lhs(ii, kk) * rhs(kk, jj);
            }
            res(ii, jj) = sum;
          }
        }
      }
    }
  }

  return res;
}
