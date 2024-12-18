
#include <stdexcept>
#include <immintrin.h>  // For potential SIMD usage
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

      // Process loop in chunks which might expose vectorization opportunities
      int k;
      for (k = 0; k <= sharedDim - 4; k += 4) {  // Assuming `sharedDim` is a multiple of 4
        sum += lhs(i, k) * rhs(k, j);
        sum += lhs(i, k + 1) * rhs(k + 1, j);
        sum += lhs(i, k + 2) * rhs(k + 2, j);
        sum += lhs(i, k + 3) * rhs(k + 3, j);
      }
      // Handle remaining elements
      for (; k < sharedDim; ++k) {
        sum += lhs(i, k) * rhs(k, j);
      }

      res(i, j) = sum;
    }
  }

  return res;
}
