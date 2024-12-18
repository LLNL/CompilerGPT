
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Optimized block sizes for modern CPU architectures
    constexpr int BLOCK_M = 96;  // Larger block for better L2 cache utilization
    constexpr int BLOCK_N = 64;  // Balanced for cache lines
    constexpr int BLOCK_K = 128; // Increased K blocking
    constexpr int MINI_M = 12;   // For AVX-512 friendly sizes
    constexpr int MINI_N = 8;    // Optimized for SIMD width
    constexpr int MICRO_K = 16;  // L1 cache oriented

    // Main loop - iterate over large blocks
    for (int bm = 0; bm < M; bm += BLOCK_M) {
        const int max_i = std::min(bm + BLOCK_M, M);
        
        for (int bn = 0; bn < N; bn += BLOCK_N) {
            const int max_j = std::min(bn + BLOCK_N, N);
            
            // K-blocking for cache optimization
            for (int bk = 0; bk < K; bk += BLOCK_K) {
                const int max_k = std::min(bk + BLOCK_K, K);

                // Mini-block processing
                for (int i = bm; i < max_i; i += MINI_M) {
                    const int mini_i_end = std::min(i + MINI_M, max_i);
                    
                    for (int j = bn; j < max_j; j += MINI_N) {
                        const int mini_j_end = std::min(j + MINI_N, max_j);

                        // Local accumulators - sized for register blocking
                        SimpleMatrix::value_type acc[MINI_M][MINI_N] = {};
                        
                        // Initialize accumulators
                        if (bk > 0) {
                            for (int mi = 0; mi < mini_i_end - i; ++mi) {
                                for (int mj = 0; mj < mini_j_end - j; ++mj) {
                                    acc[mi][mj] = res(i + mi, j + mj);
                                }
                            }
                        }

                        // Process micro-blocks of K
                        for (int k = bk; k < max_k; k += MICRO_K) {
                            const int micro_k_end = std::min(k + MICRO_K, max_k);

                            // Main computation kernel with aggressive unrolling
                            for (int uk = k; uk < micro_k_end; ++uk) {
                                // Cache rhs values for current k-iteration
                                SimpleMatrix::value_type rhs_cache[MINI_N];
                                #pragma unroll
                                for (int mj = 0; mj < mini_j_end - j; ++mj) {
                                    rhs_cache[mj] = rhs(uk, j + mj);
                                }

                                // Compute partial products with maximum unrolling
                                #pragma unroll
                                for (int mi = 0; mi < mini_i_end - i; ++mi) {
                                    const SimpleMatrix::value_type lhs_val = lhs(i + mi, uk);
                                    #pragma unroll
                                    for (int mj = 0; mj < mini_j_end - j; ++mj) {
                                        acc[mi][mj] += lhs_val * rhs_cache[mj];
                                    }
                                }
                            }
                        }

                        // Store accumulated results
                        for (int mi = 0; mi < mini_i_end - i; ++mi) {
                            for (int mj = 0; mj < mini_j_end - j; ++mj) {
                                res(i + mi, j + mj) = acc[mi][mj];
                            }
                        }
                    }
                }
            }
        }
    }

    return res;
}
