
#include <stdexcept>
#include <vector>

#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  SimpleMatrix res{lhs.rows(), rhs.columns()};
  std::vector<std::vector<int>> rhsT(rhs.columns(), std::vector<int>(rhs.rows()));

  // Transpose rhs for better cache performance
  for (int i = 0; i < rhs.rows(); ++i) {
    for (int j = 0; j < rhs.columns(); ++j) {
      rhsT[j][i] = rhs(i, j);
    }
  }

  // Perform matrix multiplication with transposed rhs
  for (int i = 0; i < lhs.rows(); ++i) {
    for (int j = 0; j < rhs.columns(); ++j) {
      int sum = 0;
      for (int k = 0; k < lhs.columns(); ++k) {
        sum += lhs(i, k) * rhsT[j][k];
      }
      res(i, j) = sum;
    }
  }

  return res;
}
