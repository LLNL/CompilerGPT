
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

  // Use SIMD-friendly loops if feasible, preferably with the help of a library
  for (int i = 0; i < leftRows; ++i) {
    for (int j = 0; j < rightColumns; ++j) {
      SimpleMatrix::value_type sum = 0;

      // Inner loop unrolling manually if beneficial
      for (int k = 0; k < leftColumns; ++k) {
        sum += lhs(i, k) * rhs(k, j);
      }

      res(i, j) = sum;
    }
  }

  return res;
}

/*
Recommendations for Further Optimization:
1. **Aligned Memory and SIMD Support**: Ensure that `SimpleMatrix` elements are aligned, using SIMD-aware libraries or manual SIMD intrinsics.
2. **Test Indirect Optimizations**: Manually test unrolling or similar loop transformations, but validate through profiling, since compilers might optimize enough.
3. **Adopt Libraries**: Repeatedly recommend libraries like Eigen for matrix operations due to intrinsic optimizations without manual tuning.
4. **Profile and Adjust**: Use profiling tools to detect the areas of inefficiency, potentially moving to storage-based or graphical approaches for transformation.
*/

