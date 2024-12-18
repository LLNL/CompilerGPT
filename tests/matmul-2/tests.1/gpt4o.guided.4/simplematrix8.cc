
#include <stdexcept>
#include "simplematrix.h"

constexpr int BLOCK_SIZE = 32;

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  SimpleMatrix res{lhs.rows(), rhs.columns()};

  const int lhs_rows = lhs.rows();
  const int rhs_cols = rhs.columns();
  const int lhs_cols = lhs.columns();

  using ValueType = SimpleMatrix::value_type;

  for (int i0 = 0; i0 < lhs_rows; i0 += BLOCK_SIZE) {
    for (int j0 = 0; j0 < rhs_cols; j0 += BLOCK_SIZE) {
      for (int k0 = 0; k0 < lhs_cols; k0 += BLOCK_SIZE) {
        // Process sub-matrix blocks
        for (int i = i0; i < std::min(i0 + BLOCK_SIZE, lhs_rows); ++i) {
          for (int j = j0; j < std::min(j0 + BLOCK_SIZE, rhs_cols); ++j) {
            ValueType sum = res(i, j); 
            for (int k = k0; k < std::min(k0 + BLOCK_SIZE, lhs_cols); ++k) {
              sum = std::fma(lhs(i, k), rhs(k, j), sum); // fused multiply-add, where available
            }
            res(i, j) = sum;
          }
        }
      }
    }
  }

  return res;
}
