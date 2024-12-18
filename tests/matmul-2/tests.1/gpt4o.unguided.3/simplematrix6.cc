
#include <stdexcept>
#include "simplematrix.h"

// Define block size for blocking technique
const int BLOCK_SIZE = 16; 

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
            ValueType sum = 0;  // Initialize sum for the block

            // Unroll the innermost loop to increase performance where possible
            int k = kk;
            int kEnd = std::min(kk + BLOCK_SIZE, lhs_cols);
            for (; k + 3 < kEnd; k+=4) {
              sum += lhs(i, k) * rhs(k, j);
              sum += lhs(i, k+1) * rhs(k+1, j);
              sum += lhs(i, k+2) * rhs(k+2, j);
              sum += lhs(i, k+3) * rhs(k+3, j);
            }
            // Handle the remaining elements
            for (; k < kEnd; ++k) {
              sum += lhs(i, k) * rhs(k, j);
            }

            res(i, j) += sum;  // Write back the sum once per element
          }
        }
      }
    }
  }

  return res;
}
