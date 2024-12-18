
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Tuned block sizes for L1/L2 cache and register file
    constexpr int BLOCK_SIZE = 32;
    constexpr int MINI_BLOCK_M = 8;
    constexpr int MINI_BLOCK_N = 4;
    constexpr int MICRO_BLOCK_K = 16;

    // Main blocking for cache levels
    for (int ii = 0; ii < M; ii += BLOCK_SIZE) {
        const int i_end = std::min(ii + BLOCK_SIZE, M);
        
        for (int jj = 0; jj < N; jj += BLOCK_SIZE) {
            const int j_end = std::min(jj + BLOCK_SIZE, N);

            // K-blocking for better cache utilization
            for (int kk = 0; kk < K; kk += MICRO_BLOCK_K) {
                const int k_end = std::min(kk + MICRO_BLOCK_K, K);

                // Register blocking
                for (int i = ii; i < i_end; i += MINI_BLOCK_M) {
                    const int mini_i_end = std::min(i + MINI_BLOCK_M, i_end);
                    
                    for (int j = jj; j < j_end; j += MINI_BLOCK_N) {
                        const int mini_j_end = std::min(j + MINI_BLOCK_N, j_end);

                        // Local accumulators in registers
                        SimpleMatrix::value_type local_sum[MINI_BLOCK_M][MINI_BLOCK_N];
                        
                        // Initialize or load existing values
                        if (kk == 0) {
                            for (int ib = 0; ib < mini_i_end - i; ++ib) {
                                for (int jb = 0; jb < mini_j_end - j; ++jb) {
                                    local_sum[ib][jb] = 0;
                                }
                            }
                        } else {
                            for (int ib = 0; ib < mini_i_end - i; ++ib) {
                                for (int jb = 0; jb < mini_j_end - j; ++jb) {
                                    local_sum[ib][jb] = res(i + ib, j + jb);
                                }
                            }
                        }

                        // Main computation kernel with aggressive unrolling
                        for (int k = kk; k < k_end; ++k) {
                            // Cache rhs values
                            SimpleMatrix::value_type rhs_cache[MINI_BLOCK_N];
                            for (int jb = 0; jb < mini_j_end - j; ++jb) {
                                rhs_cache[jb] = rhs(k, j + jb);
                            }

                            // Compute partial products with unrolled inner loop
                            for (int ib = 0; ib < mini_i_end - i; ++ib) {
                                const SimpleMatrix::value_type lhs_val = lhs(i + ib, k);
                                #pragma unroll
                                for (int jb = 0; jb < MINI_BLOCK_N; ++jb) {
                                    if (j + jb < mini_j_end) {
                                        local_sum[ib][jb] += lhs_val * rhs_cache[jb];
                                    }
                                }
                            }
                        }

                        // Store accumulated results
                        for (int ib = 0; ib < mini_i_end - i; ++ib) {
                            for (int jb = 0; jb < mini_j_end - j; ++jb) {
                                res(i + ib, j + jb) = local_sum[ib][jb];
                            }
                        }
                    }
                }
            }
        }
    }

    return res;
}
