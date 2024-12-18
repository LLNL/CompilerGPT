
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

  // Perform the matrix multiplication
  for (int i = 0; i < leftRows; ++i) {
    for (int j = 0; j < rightColumns; ++j) {
      SimpleMatrix::value_type sum = static_cast<SimpleMatrix::value_type>(0);
      for (int k = 0; k < leftColumns; ++k) {
        // Accumulate multiplication result
        sum += lhs(i, k) * rhs(k, j);
      }
      res(i, j) = sum;
    }
  }

  return res;
}

// Suggestions for implementation and improvisions:
// 1. **Leverage Libraries**: Utilize a high-performance matrix library like Eigen, Armadillo, or Blaze for optimized multiplication tasks.
//
// 2. **Matrix Storage and Access**: Ensure that matrices in `SimpleMatrix` are stored in a way that access patterns align well with cache lines. Adjust access to be cache-friendly.
//
// 3. **Data Alignment**: Use aligned memory allocations for matrix elements (e.g., `std::vector<aligned_memory>` with aligned allocators).
//
// 4. **Parallelization and Intrinsics**: Consider compiler intrinsics for loop vectorization, or intrinsics for SIMD operations if needed, potentially followed by hassle-free library options.
