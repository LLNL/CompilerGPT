
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  if (lhs.columns() != rhs.rows()) {
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};
  }

  SimpleMatrix res{lhs.rows(), rhs.columns()};

  const int lhsRows = lhs.rows();
  const int lhsCols = lhs.columns();
  const int rhsCols = rhs.columns();

  for (int i = 0; i < lhsRows; ++i) {
    for (int k = 0; k < lhsCols; ++k) {
      // Access the element of lhs only once per row-column combination.
      auto lhsValue = lhs(i, k);
      int j = 0;

      // Loop unrolling to minimize loop overhead in inner loop
      for (; j <= rhsCols - 4; j += 4) {
        res(i, j) += lhsValue * rhs(k, j);
        res(i, j + 1) += lhsValue * rhs(k, j + 1);
        res(i, j + 2) += lhsValue * rhs(k, j + 2);
        res(i, j + 3) += lhsValue * rhs(k, j + 3);
      }

      // Handle the remainder of the column elements
      for (; j < rhsCols; ++j) {
        res(i, j) += lhsValue * rhs(k, j);
      }
    }
  }

  return res;
}
