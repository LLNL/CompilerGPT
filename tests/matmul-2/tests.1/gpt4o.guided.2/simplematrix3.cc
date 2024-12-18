
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  if (lhs.columns() != rhs.rows()) {
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};
  }

  using ValueType = SimpleMatrix::value_type;

  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();

  SimpleMatrix res{lhsRows, rhsCols};
  SimpleMatrix rhsTransposed{rhsCols, lhsCols};

  // Transpose the rhs matrix
  for (int i = 0; i < rhsCols; ++i) {
    for (int j = 0; j < lhsCols; ++j) {
      rhsTransposed(i, j) = rhs(j, i);
    }
  }

  // Perform matrix multiplication
  for (int i = 0; i < lhsRows; ++i) {
    for (int j = 0; j < rhsCols; ++j) {
      ValueType sum = static_cast<ValueType>(0);
      for (int k = 0; k < lhsCols; ++k) {
        sum += lhs(i, k) * rhsTransposed(j, k);
      }
      res(i, j) = sum;
    }
  }

  return res;
}
