
#include <stdexcept>
#include "simplematrix.h"

constexpr int BLOCK_SIZE = 32; // Consider testing different block sizes

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  SimpleMatrix res{lhs.rows(), rhs.columns()};

  const int lhs_rows = lhs.rows();
  const int rhs_cols = rhs.columns();
  const int lhs_cols = lhs.columns();

  using ValueType = SimpleMatrix::value_type;

  ValueType* res_data = res.data(); // Ensure your SimpleMatrix class has a data() member function returning a pointer
  const ValueType* lhs_data = lhs.data();
  const ValueType* rhs_data = rhs.data();

  for (int i0 = 0; i0 < lhs_rows; i0 += BLOCK_SIZE) {
    for (int j0 = 0; j0 < rhs_cols; j0 += BLOCK_SIZE) {
      for (int k0 = 0; k0 < lhs_cols; k0 += BLOCK_SIZE) {
        for (int i = i0; i < std::min(i0 + BLOCK_SIZE, lhs_rows); ++i) {
          for (int j = j0; j < std::min(j0 + BLOCK_SIZE, rhs_cols); ++j) {
            ValueType sum = res_data[i * rhs_cols + j];
            #pragma GCC ivdep // Suggest vectorization if applicable
            for (int k = k0; k < std::min(k0 + BLOCK_SIZE, lhs_cols); ++k) {
              sum += lhs_data[i * lhs_cols + k] * rhs_data[k * rhs_cols + j];
            }
            res_data[i * rhs_cols + j] = sum;
          }
        }
      }
    }
  }

  return res;
}
