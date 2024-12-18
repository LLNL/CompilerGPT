
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  if (lhs.columns() != rhs.rows()) {
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};
  }

  SimpleMatrix res{lhs.rows(), rhs.columns()};

  // Reorder loops to make better use of cache.
  for (int i = 0; i < lhs.rows(); ++i) {
    for (int k = 0; k < lhs.columns(); ++k) {
      // Use a local variable for the lhs row element.
      auto lhsValue = lhs(i, k);
      for (int j = 0; j < rhs.columns(); ++j) {
        // Accumulate using a local variable to reduce method access overhead.
        res(i, j) += lhsValue * rhs(k, j);
      }
    }
  }

  return res;
}
