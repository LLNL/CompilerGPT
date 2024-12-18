
#include <stdexcept>
#include "simplematrix.h"

// Define block size; this may be adjusted based on the cache characteristics of the target system
const int BLOCK_SIZE = 32;

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  const int lhs_cols = lhs.columns();
  const int lhs_rows = lhs.rows();
  const int rhs_cols = rhs.columns();
  const int rhs_rows = rhs.rows();

  if (lhs_cols != rhs_rows)
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  using ValueType = SimpleMatrix::value_type;
  SimpleMatrix res{lhs_rows, rhs_cols};

  // Perform block matrix multiplication
  for (int ii = 0; ii < lhs_rows; ii += BLOCK_SIZE)
  {
    for (int jj = 0; jj < rhs_cols; jj += BLOCK_SIZE)
    {
      for (int kk = 0; kk < lhs_cols; kk += BLOCK_SIZE)
      {
        for (int i = ii; i < std::min(ii + BLOCK_SIZE, lhs_rows); ++i)
        {
          for (int j = jj; j < std::min(jj + BLOCK_SIZE, rhs_cols); ++j)
          {
            ValueType sum = res(i, j);

            for (int k = kk; k < std::min(kk + BLOCK_SIZE, lhs_cols); ++k)
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
