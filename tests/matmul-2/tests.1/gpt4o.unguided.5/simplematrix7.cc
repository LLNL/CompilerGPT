
#include <stdexcept>
#include "simplematrix.h"
// Remove SIMD headers due to lack of direct address access for elements

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

  // Define block size to fit working data into cache
  constexpr int blockSize = 16;  // Adjust according to platform specifics if needed

  for (int ii = 0; ii < lhsRows; ii += blockSize)
  {
    for (int jj = 0; jj < rhsCols; jj += blockSize)
    {
      for (int kk = 0; kk < lhsCols; kk += blockSize)
      {
        int iMax = std::min(ii + blockSize, lhsRows);
        int jMax = std::min(jj + blockSize, rhsCols);
        int kMax = std::min(kk + blockSize, lhsCols);

        for (int i = ii; i < iMax; ++i)
        {
          for (int j = jj; j < jMax; ++j)
          {
            ValueType sum = res(i, j); // Initialize with current value

            for (int k = kk; k < kMax; ++k)
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
