
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  // Check for dimension compatibility
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"Dimension mismatch: lhs.columns() != rhs.rows()"};

  // Create a matrix to hold the result
  SimpleMatrix res(lhs.rows(), rhs.columns());
  
  // Pre-compute dimensions and reduce function calls
  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();

  // Using SimpleMatrix::value_type to ensure type consistency
  using ValueType = typename SimpleMatrix::value_type;

  // Optimize using cache blocking
  constexpr int blockSize = 64 / sizeof(ValueType); // estimate block size based on cache line

  for (int ii = 0; ii < lhsRows; ii += blockSize)
  {
    for (int jj = 0; jj < rhsCols; jj += blockSize)
    {
      for (int kk = 0; kk < lhsCols; kk += blockSize)
      {
        // Work on blocks
        for (int i = ii; i < std::min(ii + blockSize, lhsRows); ++i)
        {
          for (int j = jj; j < std::min(jj + blockSize, rhsCols); ++j)
          {
            ValueType sum = res(i, j);  // Start with existing value
            for (int k = kk; k < std::min(kk + blockSize, lhsCols); ++k)
            {
              sum += lhs(i, k) * rhs(k, j);
            }
            res(i, j) = sum;
          }
        }
      }
    }
  }

  return res;
}
