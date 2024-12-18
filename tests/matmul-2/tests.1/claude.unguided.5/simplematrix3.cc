
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Cache line optimization - process in blocks
    constexpr int BLOCK_SIZE = 32;
    constexpr int MINI_BLOCK = 4;  // For register blocking

    for (int ii = 0; ii < M; ii += BLOCK_SIZE) {
        const int i_end = std::min(ii + BLOCK_SIZE, M);
        for (int jj = 0; jj < N; jj += BLOCK_SIZE) {
            const int j_end = std::min(jj + BLOCK_SIZE, N);
            
            // Mini-block processing for better register utilization
            for (int i = ii; i < i_end; i += MINI_BLOCK) {
                const int mini_end = std::min(i + MINI_BLOCK, i_end);
                for (int j = jj; j < j_end; ++j) {
                    // Pre-initialize accumulators
                    SimpleMatrix::value_type sum[MINI_BLOCK] = {0};
                    
                    // Main computation loop with potential vectorization
                    for (int k = 0; k < K; ++k) {
                        const SimpleMatrix::value_type r_val = rhs(k,j);
                        // Unrolled mini-block
                        for (int ib = i; ib < mini_end; ++ib) {
                            sum[ib - i] += lhs(ib,k) * r_val;
                        }
                    }
                    
                    // Store results
                    for (int ib = i; ib < mini_end; ++ib) {
                        res(ib,j) = sum[ib - i];
                    }
                }
            }
        }
    }

    return res;
}
