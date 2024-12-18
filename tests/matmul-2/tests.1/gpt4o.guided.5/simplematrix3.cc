
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
      // Using value_type for sum
      SimpleMatrix::value_type sum1 = 0;
      SimpleMatrix::value_type sum2 = 0;

      // Manually unrolling the loop two steps at a time
      int k;
      for (k = 0; k + 1 < sharedDim; k += 2) {
        sum1 += lhs(i, k) * rhs(k, j);
        sum2 += lhs(i, k + 1) * rhs(k + 1, j);
      }
      if (k < sharedDim) { // Handle the possibly odd remaining element
        sum1 += lhs(i, k) * rhs(k, j);
      }
      
      res(i, j) = sum1 + sum2;
    }
  }

  return res;
}
