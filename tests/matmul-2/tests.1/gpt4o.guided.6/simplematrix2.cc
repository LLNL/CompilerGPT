
#include <stdexcept>
#include <vector>
#include "simplematrix.h"

// Assuming SimpleMatrix provides a typedef for its value type
using ValueType = SimpleMatrix::value_type;

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  SimpleMatrix res{lhs.rows(), rhs.columns()};

  // Transpose rhs for better cache performance
  std::vector<std::vector<ValueType>> rhsT(rhs.columns(), std::vector<ValueType>(rhs.rows()));
  for (int i = 0; i < rhs.rows(); ++i) {
    for (int j = 0; j < rhs.columns(); ++j) {
      rhsT[j][i] = rhs(i, j);
    }
  }

  // Perform matrix multiplication with transposed rhs
  for (int i = 0; i < lhs.rows(); ++i) {
    for (int j = 0; j < rhs.columns(); ++j) {
      ValueType sum = 0;
      for (int k = 0; k < lhs.columns(); ++k) {
        sum += lhs(i, k) * rhsT[j][k];
      }
      res(i, j) = sum;
    }
  }

  return res;
}
