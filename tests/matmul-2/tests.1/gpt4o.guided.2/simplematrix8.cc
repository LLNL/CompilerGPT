
#include <stdexcept>
#include <immintrin.h>  // Include for AVX intrinsics
#include "simplematrix.h"

// Assuming the SimpleMatrix uses double as its value_type; if it uses long double, consider constraints and adjustments.

SimpleMatrix operator*(const SimpleMatrix &lhs, const SimpleMatrix &rhs) {
    // Check matrix dimensions for validity
    if (lhs.columns() != rhs.rows()) {
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};
    }

    using ValueType = SimpleMatrix::value_type;
    static_assert(std::is_same_v<ValueType, double>, "Expecting double precision arithmetic for AVX intrinsics.");

    int lhsRows = lhs.rows();
    int lhsCols = lhs.columns();
    int rhsCols = rhs.columns();

    SimpleMatrix res{lhsRows, rhsCols};

    // Transpose the rhs matrix for better memory access patterns
    SimpleMatrix rhsTransposed(rhsCols, lhsCols);
    for (int i = 0; i < lhsCols; ++i) {
        for (int j = 0; j < rhsCols; ++j) {
            rhsTransposed(j, i) = rhs(i, j);
        }
    }

    for (int i = 0; i < lhsRows; ++i) {
        for (int j = 0; j < rhsCols; ++j) {
            __m256d sums = _mm256_setzero_pd();
            
            // Compute over blocks of 4 doubles, assuming index checks are managed
            for (int k = 0; k <= lhsCols - 4; k += 4) {
                // Load a block from lhs
                __m256d lhsBlock = _mm256_loadu_pd(&lhs(i, k));
                
                // Load a block from transposed rhs
                __m256d rhsBlock = _mm256_loadu_pd(&rhsTransposed(j, k));

                // Perform SIMD multiplication and accumulate
                sums = _mm256_fmadd_pd(lhsBlock, rhsBlock, sums);
            }

            // Accumulate the results in the sum vector
            double sumArray[4];
            _mm256_storeu_pd(sumArray, sums);
            ValueType finalSum = sumArray[0] + sumArray[1] + sumArray[2] + sumArray[3];

            // Add any remaining elements (less than 4)
            for (int k = (lhsCols / 4) * 4; k < lhsCols; ++k) {
                finalSum += lhs(i, k) * rhsTransposed(j, k);
            }

            res(i, j) = finalSum;
        }
    }

    return res;
}
