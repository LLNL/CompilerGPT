
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  if (lhs.columns() != rhs.rows()) {
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};
  }

  SimpleMatrix res(lhs.rows(), rhs.columns());

  const int lhsRows = lhs.rows();
  const int rhsColumns = rhs.columns();
  const int sharedDim = lhs.columns();

  for (int i = 0; i < lhsRows; ++i) {
    for (int j = 0; j < rhsColumns; ++j) {
      SimpleMatrix::value_type sum = 0;

      // Access data in ways that may improve caching behavior
      for (int k = 0; k < sharedDim; ++k) {
        sum += lhs(i, k) * rhs(k, j);
      }

      res(i, j) = sum;
    }
  }

  return res;
}
