
#include <stdexcept>
#include <algorithm>
#include "simplematrix.h"

// Define a block size, typically a multiple of the cache line size or experimentally determined.
constexpr int BLOCK_SIZE = 32;

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  int lhsRows = lhs.rows();
  int lhsCols = lhs.columns();
  int rhsCols = rhs.columns();
  
  if (lhsCols != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  SimpleMatrix res{lhsRows, rhsCols};

  // Iterate over sub-blocks of (lhsRows x rhsCols) matrix res
  for (int i = 0; i < lhsRows; i += BLOCK_SIZE)
  {
    for (int j = 0; j < rhsCols; j += BLOCK_SIZE)
    {
      // Iterate over sub-blocks of (lhsCols) shared dimension
      for (int k = 0; k < lhsCols; k += BLOCK_SIZE)
      {
        // Compute block boundaries
        int iMax = std::min(i + BLOCK_SIZE, lhsRows);
        int jMax = std::min(j + BLOCK_SIZE, rhsCols);
        int kMax = std::min(k + BLOCK_SIZE, lhsCols);

        // Block computation
        for (int ib = i; ib < iMax; ++ib)
        {
          for (int jb = j; jb < jMax; ++jb)
          {
            SimpleMatrix::value_type sum = 0;

            for (int kb = k; kb < kMax; ++kb)
            {
              sum += lhs(ib, kb) * rhs(kb, jb);
            }

            res(ib, jb) += sum;
          }
        }
      }
    }
  }

  return res;
}
