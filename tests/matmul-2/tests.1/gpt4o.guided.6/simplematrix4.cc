
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

  // Determine an optimal block size: for simplicity, starting with 64 or adjust to 32 if beneficial
  const int blockSize = 32;

  // Use blocking to optimize cache performance
  for (int ii = 0; ii < lhsRows; ii += blockSize) {
    for (int jj = 0; jj < rhsCols; jj += blockSize) {
      for (int kk = 0; kk < lhsCols; kk += blockSize) {

        // Operate over blocks
        for (int i = ii; i < std::min(ii + blockSize, lhsRows); ++i) {
          for (int j = jj; j < std::min(jj + blockSize, rhsCols); ++j) {
            ValueType sum = 0; // Start with sum assuming res(i,j) was uninitiated

            for (int k = kk; k < std::min(kk + blockSize, lhsCols); ++k) {
              sum += lhs(i, k) * rhsT[j][k];
            }
            // Direct placement of the sum as we construct new values
            res(i, j) += sum;
          }
        }
      }
    }
  }

  return res;
}
