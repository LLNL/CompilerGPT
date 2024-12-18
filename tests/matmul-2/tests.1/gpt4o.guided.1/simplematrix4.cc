
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  int leftRows = lhs.rows();
  int leftColumns = lhs.columns();
  int rightColumns = rhs.columns();
  
  SimpleMatrix res{leftRows, rightColumns};

  // Ensure lhs and rhs offer necessary aligned storage access if relevant
  // This is critical if we want to fully leverage vectorization features.

  for (int i = 0; i < leftRows; ++i)
  {
    for (int j = 0; j < rightColumns; ++j)
    {
      SimpleMatrix::value_type sum = static_cast<SimpleMatrix::value_type>(0);

      for (int k = 0; k < leftColumns; ++k)
      {
        // Potentially unroll or leverage intrinsic functions if hardware supports
        // Specific unrolling or use of SIMD intrinsics could replace this loop
        sum += lhs(i, k) * rhs(k, j);
      }
      res(i, j) = sum;
    }
  }

  return res;
}
