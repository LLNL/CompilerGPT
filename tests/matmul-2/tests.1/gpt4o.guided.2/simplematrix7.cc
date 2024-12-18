
#include <stdexcept>
#include <algorithm>
#include <immintrin.h>  // For SIMD intrinsics, e.g., AVX or SSE, depending on your target hardware
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    // Ensure matrices can be multiplied
    if (lhs.columns() != rhs.rows()) {
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};
    }

    using ValueType = SimpleMatrix::value_type;
    int lhsRows = lhs.rows();
    int lhsCols = lhs.columns();
    int rhsCols = rhs.columns();

    SimpleMatrix res{lhsRows, rhsCols};

    // Transpose the rhs matrix to optimize row-wise access
    SimpleMatrix rhsTransposed(rhsCols, lhsCols);
    for (int i = 0; i < lhsCols; ++i) {
        for (int j = 0; j < rhsCols; ++j) {
            rhsTransposed(j, i) = rhs(i, j);
        }
    }

    // Use SIMD for block multiplication
    const int SIMD_WIDTH = 8; // Assume AVX2 with 256-bit registers (8 doubles)
    for (int i = 0; i < lhsRows; ++i) {
        for (int j = 0; j < rhsCols; ++j) {
            __m256d sums = _mm256_setzero_pd();
            for (int k = 0; k < lhsCols; k += SIMD_WIDTH) {
                // Load a block from lhs
                __m256d lhsBlock = _mm256_loadu_pd(&lhs(i, k));

                // Load a block from transposed rhs
                __m256d rhsBlock = _mm256_loadu_pd(&rhsTransposed(j, k));

                // Perform SIMD multiplication and accumulate
                sums = _mm256_fmadd_pd(lhsBlock, rhsBlock, sums); // fused multiply-add
            }
            // Collapse sum vector to a scalar
            ValueType* sumArray = (ValueType*)&sums;
            res(i, j) = sumArray[0] + sumArray[1] + sumArray[2] + sumArray[3];
        }
    }

    return res;
}
