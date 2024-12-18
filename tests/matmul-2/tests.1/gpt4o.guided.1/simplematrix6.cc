
#include <stdexcept>
#include "simplematrix.h" // SimpleMatrix must handle aligned storage and access

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
      SimpleMatrix::value_type sum = static_cast<SimpleMatrix::value_type>(0);

      for (int k = 0; k < leftColumns; ++k) {
        sum += lhs(i, k) * rhs(k, j);
      }

      res(i, j) = sum;
    }
  }

  return res;
}

// Suggestions for Implementation:
// 1. **Aligned Storage**: Ensure `SimpleMatrix` uses aligned memory techniques like `std::aligned_alloc`, enhancing SIMD performance.
// 2. **Use Libraries**: Adopt matrix libraries like Eigen or Armadillo as they inherently manage memory layouts for optimal vectorization.
// 3. **Explore SIMD Intrinsics**: Investigate manual SIMD usage through intrinsics or compiler hints, especially if libraries can't be used.
