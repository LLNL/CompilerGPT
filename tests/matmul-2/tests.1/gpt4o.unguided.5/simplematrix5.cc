
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  // Check for dimension compatibility
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"Dimension mismatch: lhs.columns() != rhs.rows()"};

  // Create a matrix to hold the result
  SimpleMatrix res(lhs.rows(), rhs.columns());
  
  // Pre-compute dimensions and ensure type consistency
  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();
  
  using ValueType = typename SimpleMatrix::value_type;

  // Define block size to work with typical cache line sizes
  constexpr int blockSize = 16;  // Choose a small block size to improve cache locality

  for (int ii = 0; ii < lhsRows; ii += blockSize)
  {
    for (int jj = 0; jj < rhsCols; jj += blockSize)
    {
      for (int kk = 0; kk < lhsCols; kk += blockSize)
      {
        // Process blocks
        int iMax = std::min(ii + blockSize, lhsRows);
        int jMax = std::min(jj + blockSize, rhsCols);
        int kMax = std::min(kk + blockSize, lhsCols);

        for (int i = ii; i < iMax; ++i)
        {
          for (int j = jj; j < jMax; ++j)
          {
            ValueType sum = res(i, j);

            // Unroll the innermost loop, as an example unroll factor of 4
            int k = kk;
            for (; k <= kMax - 4; k += 4)
            {
              sum += lhs(i, k) * rhs(k, j);
              sum += lhs(i, k + 1) * rhs(k + 1, j);
              sum += lhs(i, k + 2) * rhs(k + 2, j);
              sum += lhs(i, k + 3) * rhs(k + 3, j);
            }
            // Handle remaining iterations if kMax is not divisible by 4
            for (; k < kMax; ++k)
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
