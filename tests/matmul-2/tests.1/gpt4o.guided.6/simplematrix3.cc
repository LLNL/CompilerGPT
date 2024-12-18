
#include <stdexcept>
#include <vector>
#include "simplematrix.h"

using ValueType = SimpleMatrix::value_type;

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();
  
  if (lhsCols != rhs.rows()) {
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};
  }

  SimpleMatrix res{lhsRows, rhsCols};

  // Transpose rhs for better cache performance
  std::vector<std::vector<ValueType>> rhsT(rhsCols, std::vector<ValueType>(lhsCols));
  for (int i = 0; i < lhsCols; ++i) {
    for (int j = 0; j < rhsCols; ++j) {
      rhsT[j][i] = rhs(i, j);
    }
  }

  // Manual blocking parameters
  const int blockSize = 64; // An experimentally derived value based on cache size

  // Perform blocked matrix multiplication
  for (int ii = 0; ii < lhsRows; ii += blockSize) {
    for (int jj = 0; jj < rhsCols; jj += blockSize) {
      for (int kk = 0; kk < lhsCols; kk += blockSize) {
        
        // Block multiplicative operation
        for (int i = ii; i < std::min(ii + blockSize, lhsRows); ++i) {
          for (int j = jj; j < std::min(jj + blockSize, rhsCols); ++j) {
            ValueType sum = res(i, j);

            for (int k = kk; k < std::min(kk + blockSize, lhsCols); ++k) {
              sum += lhs(i, k) * rhsT[j][k];
            }
            res(i, j) = sum;
          }
        }

      }
    }
  }

  return res;
}
