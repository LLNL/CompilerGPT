
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  SimpleMatrix res{lhs.rows(), rhs.columns()};

  // Hoisting and rearranging invariants outside of loops
  const int lhs_rows = lhs.rows();
  const int rhs_cols = rhs.columns();
  const int lhs_cols = lhs.columns();

  using ValueType = SimpleMatrix::value_type;

  for (int i = 0; i < lhs_rows; ++i) {
    for (int j = 0; j < rhs_cols; ++j) {
      ValueType sum = static_cast<ValueType>(0);  // Use SimpleMatrix::value_type for sum
      for (int k = 0; k < lhs_cols; ++k) {
        sum += lhs(i, k) * rhs(k, j);
      }
      res(i, j) = sum;
    }
  }

  return res;
}
