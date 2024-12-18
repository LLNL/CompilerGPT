
#include <stdexcept>
#include "simplematrix.h" // Assuming SimpleMatrix has aligned storage and efficient access

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs)
{
  if (lhs.columns() != rhs.rows())
    throw std::runtime_error{"lhs.columns() != rhs.rows()"};

  int leftRows = lhs.rows();
  int leftColumns = lhs.columns();
  int rightColumns = rhs.columns();
  
  SimpleMatrix res{leftRows, rightColumns};

  // Consider employing a high-performance matrix library such as Eigen
  for (int i = 0; i < leftRows; ++i) {
    for (int j = 0; j < rightColumns; ++j) {
      SimpleMatrix::value_type sum = 0;

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
1. Use Eigen, Armadillo, or similar libraries: These libraries automatically take advantage of SIMD and align data for best performance.
2. Intrinsic Support: Consider using compiler intrinsics (like AVX or SSE) for direct SIMD operations if the performance gain is substantial and justified.
3. Profile-driven Development: Run profiling to identify actual hot paths and data movement inefficiencies for fine-tuning.
4. Check Alignment and Storage: Ensure stored matrix data is aligned, using `std::aligned_alloc` or similar strategies for better memory access patterns.
*/
