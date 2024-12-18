
#include <stdexcept>
#include "simplematrix.h"  // Assuming this file defines SimpleMatrix appropriately

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  // Ensure matrices can be multiplied
  if (lhs.columns() != rhs.rows()) {
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};
  }

  using ValueType = SimpleMatrix::value_type;  // Assuming SimpleMatrix defines a nested value_type

  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();

  SimpleMatrix res{lhsRows, rhsCols};

  for (int i = 0; i < lhsRows; ++i) {
    for (int j = 0; j < rhsCols; ++j) {
      ValueType sum = static_cast<ValueType>(0);
      for (int k = 0; k < lhsCols; ++k) {
        // Assuming operator() correctly accesses the matrix elements
        sum += lhs(i, k) * rhs(k, j);
      }
      res(i, j) = sum;
    }
  }

  return res;
}
