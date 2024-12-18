
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  if (lhs.columns() != rhs.rows()) {
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};
  }

  using ValueType = SimpleMatrix::value_type;
  const int BLOCK_SIZE = 32; // Experimenting with smaller blocks to see cache fit improvement

  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();

  SimpleMatrix res{lhsRows, rhsCols};

  // Initialize result matrix: Can help data locality
  for (int i = 0; i < lhsRows; ++i) {
    for (int j = 0; j < rhsCols; ++j) {
      res(i, j) = 0; // Ensuring initialization outside inner loops
    }
  }

  for (int i = 0; i < lhsRows; i += BLOCK_SIZE) {
    for (int j = 0; j < rhsCols; j += BLOCK_SIZE) {
      for (int k = 0; k < lhsCols; k += BLOCK_SIZE) {
        for (int ii = i; ii < std::min(i + BLOCK_SIZE, lhsRows); ++ii) {
          for (int jj = j; jj < std::min(j + BLOCK_SIZE, rhsCols); ++jj) {
            ValueType sum = 0;
            for (int kk = k; kk < std::min(k + BLOCK_SIZE, lhsCols); ++kk) {
              // Move accumulation operation to reduce register usage
              sum += lhs(ii, kk) * rhs(kk, jj);
            }
            res(ii, jj) += sum;
          }
        }
      }
    }
  }

  return res;
}
