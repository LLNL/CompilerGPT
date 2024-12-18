
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  // Ensure matrices can be multiplied
  if (lhs.columns() != rhs.rows()) {
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};
  }

  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();

  SimpleMatrix res{lhsRows, rhsCols};

  // Iterate over the result matrix using a cache-friendly loop order
  for (int i = 0; i < lhsRows; ++i) {
    for (int j = 0; j < rhsCols; ++j) {
      double sum = 0.0;
      for (int k = 0; k < lhsCols; ++k) {
        sum += lhs(i, k) * rhs(k, j);
      }
      res(i, j) = sum;
    }
  }

  return res;
}
