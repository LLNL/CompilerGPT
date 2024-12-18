
#include <stdexcept>
#include "simplematrix.h"
#include <algorithm>
#include <emmintrin.h> // Include SSE2 intrinsic headers for SIMD

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
  constexpr int blockSize = 64 / sizeof(ValueType);  // Choose based on cache line size

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
          for (int j = jj; j < jMax; j += 2) // Unroll the outer loop by a factor of 2
          {
            __m128d sum = _mm_setzero_pd(); // SIMD register for storing double

            for (int k = kk; k < kMax; k += 4) // Keeping in sync with data alignment
            {
              __m128d lhsVals1 = _mm_loadu_pd(&lhs(i, k));      // Load lhs values
              __m128d lhsVals2 = _mm_loadu_pd(&lhs(i, k + 2));  // Load next set of lhs values

              __m128d rhsVals1 = _mm_set_pd(rhs(k+1, j), rhs(k, j));      // Load corresponding rhs values
              __m128d rhsVals2 = _mm_set_pd(rhs(k+3, j), rhs(k+2, j));    // Load next pairs of rhs

              sum = _mm_add_pd(sum, _mm_mul_pd(lhsVals1, rhsVals1)); // Multiply-add
              sum = _mm_add_pd(sum, _mm_mul_pd(lhsVals2, rhsVals2)); // Multiply-add
            }

            // Add to res and handle any remaining elements
            ValueType resSum = res(i, j) + ((sum[0] + sum[1]));

            // Add remaining elements if needed
            for (int k = kMax - (kMax % 4); k < kMax; ++k)
            {
              resSum += lhs(i, k) * rhs(k, j);
            }

            res(i, j) = resSum;

            // Assuming symmetric unrolling, compute for j+1 if jMax is even
            if (j+1 < jMax)
            {
              ValueType resSum2 = 0;
              for (int k = kk; k < kMax; ++k)
              {
                resSum2 += lhs(i, k) * rhs(k, j+1);
              }
              res(i, j+1) = resSum2;
            }
          }
        }
      }
    }
  }

  return res;
}
