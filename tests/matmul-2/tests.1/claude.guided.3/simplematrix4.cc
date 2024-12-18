
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};
    using value_type = SimpleMatrix::value_type;

    // Small mini-block for L1 cache
    constexpr int MINI_BLOCK = 4;

    for (int i = 0; i < M; ++i) {
        // Process output matrix row by row
        for (int j = 0; j < N; j += MINI_BLOCK) {
            // Initialize accumulators
            value_type sum[MINI_BLOCK] = {0};
            const int jEnd = std::min(j + MINI_BLOCK, N);
            
            // Single pass through K dimension
            for (int k = 0; k < K; ++k) {
                const value_type lhs_ik = lhs(i, k);
                
                // Manual unroll for better register reuse
                #pragma clang loop unroll(enable)
                for (int jj = j; jj < jEnd; ++jj) {
                    sum[jj - j] += lhs_ik * rhs(k, jj);
                }
            }
            
            // Store results
            #pragma clang loop unroll(enable)
            for (int jj = j; jj < jEnd; ++jj) {
                res(i, jj) = sum[jj - j];
            }
        }
    }

    return res;
}
