
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Optimized blocking parameters
    constexpr int L3_BLOCK_M = 240;  // L3 cache blocking
    constexpr int L3_BLOCK_N = 192;
    constexpr int L3_BLOCK_K = 512;

    constexpr int L2_BLOCK_M = 60;   // L2 cache blocking
    constexpr int L2_BLOCK_N = 64;
    constexpr int L2_BLOCK_K = 128;

    constexpr int L1_BLOCK_M = 20;   // L1 cache blocking
    constexpr int L1_BLOCK_N = 32;
    constexpr int L1_BLOCK_K = 32;

    constexpr int REG_BLOCK_M = 5;   // Register blocking
    constexpr int REG_BLOCK_N = 8;   // Optimized for AVX2/AVX-512

    // L3 cache blocking
    for (int bm3 = 0; bm3 < M; bm3 += L3_BLOCK_M) {
        const int max_i_l3 = std::min(bm3 + L3_BLOCK_M, M);
        
        for (int bn3 = 0; bn3 < N; bn3 += L3_BLOCK_N) {
            const int max_j_l3 = std::min(bn3 + L3_BLOCK_N, N);
            
            // L2 cache blocking
            for (int bm2 = bm3; bm2 < max_i_l3; bm2 += L2_BLOCK_M) {
                const int max_i_l2 = std::min(bm2 + L2_BLOCK_M, max_i_l3);
                
                for (int bn2 = bn3; bn2 < max_j_l3; bn2 += L2_BLOCK_N) {
                    const int max_j_l2 = std::min(bn2 + L2_BLOCK_N, max_j_l3);

                    // K-dimension blocking
                    for (int bk = 0; bk < K; bk += L2_BLOCK_K) {
                        const int max_k_l2 = std::min(bk + L2_BLOCK_K, K);

                        // L1 cache blocking
                        for (int i = bm2; i < max_i_l2; i += L1_BLOCK_M) {
                            const int max_i_l1 = std::min(i + L1_BLOCK_M, max_i_l2);
                            
                            for (int j = bn2; j < max_j_l2; j += L1_BLOCK_N) {
                                const int max_j_l1 = std::min(j + L1_BLOCK_N, max_j_l2);

                                // Register blocking
                                for (int ii = i; ii < max_i_l1; ii += REG_BLOCK_M) {
                                    const int max_i_reg = std::min(ii + REG_BLOCK_M, max_i_l1);
                                    
                                    for (int jj = j; jj < max_j_l1; jj += REG_BLOCK_N) {
                                        const int max_j_reg = std::min(jj + REG_BLOCK_N, max_j_l1);

                                        // Register accumulators
                                        SimpleMatrix::value_type acc[REG_BLOCK_M][REG_BLOCK_N] = {};

                                        // Initialize accumulators
                                        if (bk > 0) {
                                            #pragma unroll
                                            for (int ri = 0; ri < max_i_reg - ii; ++ri) {
                                                #pragma unroll
                                                for (int rj = 0; rj < max_j_reg - jj; ++rj) {
                                                    acc[ri][rj] = res(ii + ri, jj + rj);
                                                }
                                            }
                                        }

                                        // Process K in L1 cache blocks
                                        for (int k = bk; k < max_k_l2; k += L1_BLOCK_K) {
                                            const int max_k = std::min(k + L1_BLOCK_K, max_k_l2);

                                            // Main computation kernel
                                            for (int kk = k; kk < max_k; ++kk) {
                                                SimpleMatrix::value_type rhs_cache[REG_BLOCK_N];
                                                SimpleMatrix::value_type lhs_cache[REG_BLOCK_M];

                                                // Prefetch next iteration
                                                if (kk + 1 < max_k) {
                                                    #pragma unroll
                                                    for (int ri = 0; ri < max_i_reg - ii; ++ri) {
                                                        __builtin_prefetch(&lhs(ii + ri, kk + 1));
                                                    }
                                                    #pragma unroll
                                                    for (int rj = 0; rj < max_j_reg - jj; ++rj) {
                                                        __builtin_prefetch(&rhs(kk + 1, jj + rj));
                                                    }
                                                }

                                                // Cache current values
                                                #pragma unroll
                                                for (int ri = 0; ri < max_i_reg - ii; ++ri) {
                                                    lhs_cache[ri] = lhs(ii + ri, kk);
                                                }
                                                #pragma unroll
                                                for (int rj = 0; rj < max_j_reg - jj; ++rj) {
                                                    rhs_cache[rj] = rhs(kk, jj + rj);
                                                }

                                                // Compute partial products
                                                #pragma unroll
                                                for (int ri = 0; ri < max_i_reg - ii; ++ri) {
                                                    #pragma unroll
                                                    for (int rj = 0; rj < max_j_reg - jj; ++rj) {
                                                        acc[ri][rj] += lhs_cache[ri] * rhs_cache[rj];
                                                    }
                                                }
                                            }
                                        }

                                        // Store results
                                        #pragma unroll
                                        for (int ri = 0; ri < max_i_reg - ii; ++ri) {
                                            #pragma unroll
                                            for (int rj = 0; rj < max_j_reg - jj; ++rj) {
                                                res(ii + ri, jj + rj) = acc[ri][rj];
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return res;
}
