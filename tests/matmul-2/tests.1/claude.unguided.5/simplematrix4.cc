
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Optimize for common matrix sizes and cache lines
    constexpr int BLOCK_SIZE = 32;
    constexpr int MINI_BLOCK_M = 4;
    constexpr int MINI_BLOCK_N = 4;

    // Process blocks of the output matrix
    for (int ii = 0; ii < M; ii += BLOCK_SIZE) {
        const int i_end = std::min(ii + BLOCK_SIZE, M);
        
        for (int jj = 0; jj < N; jj += BLOCK_SIZE) {
            const int j_end = std::min(jj + BLOCK_SIZE, N);

            // Process mini-blocks for better register utilization
            for (int i = ii; i < i_end; i += MINI_BLOCK_M) {
                const int mini_i_end = std::min(i + MINI_BLOCK_M, i_end);
                
                for (int j = jj; j < j_end; j += MINI_BLOCK_N) {
                    const int mini_j_end = std::min(j + MINI_BLOCK_N, j_end);
                    
                    // Local accumulator matrix for register blocking
                    SimpleMatrix::value_type local_sum[MINI_BLOCK_M][MINI_BLOCK_N] = {};

                    // Main computation kernel
                    for (int k = 0; k < K; ++k) {
                        // Cache lhs values for current k
                        SimpleMatrix::value_type lhs_cache[MINI_BLOCK_M];
                        for (int ib = i; ib < mini_i_end; ++ib) {
                            lhs_cache[ib - i] = lhs(ib, k);
                        }

                        // Cache rhs values for current k
                        SimpleMatrix::value_type rhs_cache[MINI_BLOCK_N];
                        for (int jb = j; jb < mini_j_end; ++jb) {
                            rhs_cache[jb - j] = rhs(k, jb);
                        }

                        // Compute mini-block with cached values
                        for (int ib = 0; ib < mini_i_end - i; ++ib) {
                            const SimpleMatrix::value_type lhs_val = lhs_cache[ib];
                            for (int jb = 0; jb < mini_j_end - j; ++jb) {
                                local_sum[ib][jb] += lhs_val * rhs_cache[jb];
                            }
                        }
                    }

                    // Store results back to matrix
                    for (int ib = 0; ib < mini_i_end - i; ++ib) {
                        for (int jb = 0; jb < mini_j_end - j; ++jb) {
                            res(i + ib, j + jb) = local_sum[ib][jb];
                        }
                    }
                }
            }
        }
    }

    return res;
}
