
#include <stdexcept>
#include "simplematrix.h"
#include <algorithm>
#include <vector>

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  // Verify dimension compatibility
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"Dimension mismatch: lhs.columns() != rhs.rows()"};

  // Initialize result matrix
  SimpleMatrix res(lhs.rows(), rhs.columns());
  
  // Pre-compute matrix sizes
  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();

  using ValueType = typename SimpleMatrix::value_type;

  // Attempt to optimize cache usage, by iterating in a cache-friendly order
  for (int i = 0; i < lhsRows; i += 1)
  {
    for (int j = 0; j < rhsCols; j += 1)
    {
      ValueType sum = 0; // Initialize the sum for each cell

      // Rearrange the innermost loop to make use of caching effects
      for (int k = 0; k < lhsCols; ++k)
      {
        sum += lhs(i, k) * rhs(k, j);
      }

      res(i, j) = sum; // Assign computed value to the result matrix
    }
  }

  return res;
}
