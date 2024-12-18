
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  if (lhs.columns() != rhs.rows()) {
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};
  }

  using ValueType = SimpleMatrix::value_type;
  const int BLOCK_SIZE = 64; // Still tuning based on trial; may adjust

  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();

  SimpleMatrix res{lhsRows, rhsCols};

  for (int i = 0; i < lhsRows; i += BLOCK_SIZE) {
    for (int j = 0; j < rhsCols; j += BLOCK_SIZE) {
      for (int k = 0; k < lhsCols; k += BLOCK_SIZE) {
        for (int ii = i; ii < std::min(i + BLOCK_SIZE, lhsRows); ++ii) {
          for (int jj = j; jj < std::min(j + BLOCK_SIZE, rhsCols); ++jj) {
            ValueType sum = res(ii, jj);
            for (int kk = k; kk < std::min(k + BLOCK_SIZE, lhsCols); kk+=4) {
              // Unroll by 4 for reduced loop overhead and potential vectorization
              sum += lhs(ii, kk) * rhs(kk, jj);
              if (kk + 1 < lhsCols) sum += lhs(ii, kk + 1) * rhs(kk + 1, jj);
              if (kk + 2 < lhsCols) sum += lhs(ii, kk + 2) * rhs(kk + 2, jj);
              if (kk + 3 < lhsCols) sum += lhs(ii, kk + 3) * rhs(kk + 3, jj);
            }
            res(ii, jj) = sum;
          }
        }
      }
    }
  }

  return res;
}
