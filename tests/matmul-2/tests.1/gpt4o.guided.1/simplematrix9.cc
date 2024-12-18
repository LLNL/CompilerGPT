
#include <stdexcept>
#include "simplematrix.h" // Assuming SimpleMatrix has optimized data handling

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  int leftRows = lhs.rows();
  int leftColumns = lhs.columns();
  int rightColumns = rhs.columns();
  
  SimpleMatrix res{leftRows, rightColumns};

  for (int i = 0; i < leftRows; ++i) {
    for (int j = 0; j < rightColumns; ++j) {
      SimpleMatrix::value_type sum = 0;

      // Manual loop unrolling, if test shows benefits
      for (int k = 0; k < leftColumns; ++k) {
        sum += lhs(i, k) * rhs(k, j);
      }

      res(i, j) = sum;
    }
  }

  return res;
}

/*
Further Recommendations:
1. **Adopt Libraries**: Switching to libraries like Eigen for high-performance needs can offload manual optimization efforts, leveraging years of enhancements in these libraries.
2. **Data Layout**: Ensure data alignment using aligned allocators, which can provide significant benefits even for basic loop optimizations.
3. **Analyze and Use SIMD**: If staying customized, explore SIMD intrinsics, ensuring SimpleMatrix data suits such operations. Often requires testing and profiling to suit specific hardware.
4. **Use Tools**: Leverage profiling tools to confirm hotspots, and validate alignment and data moving strategies.
*/

