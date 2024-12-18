
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  SimpleMatrix res{lhs.rows(), rhs.columns()};

  // Hoisting and rearranging lower bound checks outside the innermost loop for invariants
  const int lhs_rows = lhs.rows();
  const int rhs_cols = rhs.columns();
  const int lhs_cols = lhs.columns();

  for (int i = 0; i < lhs_rows; ++i) {
    for (int j = 0; j < rhs_cols; ++j) {
      double sum = 0.0; // Use a temporary sum variable to reduce accessing the result and enable better loop optimizations
      for (int k = 0; k < lhs_cols; ++k) {
        sum += lhs(i, k) * rhs(k, j);
      }
      res(i, j) = sum; // Set the result after summing to improve cache locality
    }
  }

  return res;
}
