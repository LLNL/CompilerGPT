
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Cache-optimized block sizes
    constexpr int L2_BLOCK_M = 120;  // L2 cache blocking
    constexpr int L2_BLOCK_N = 96;
    constexpr int L2_BLOCK_K = 256;
    
    constexpr int L1_BLOCK_M = 24;   // L1 cache blocking
    constexpr int L1_BLOCK_N = 16;
    constexpr int L1_BLOCK_K = 32;
    
    constexpr int REG_BLOCK_M = 6;   // Register blocking
    constexpr int REG_BLOCK_N = 8;

    // Outer blocking for L2 cache
    for (int bm = 0; bm < M; bm += L2_BLOCK_M) {
        const int max_i_l2 = std::min(bm + L2_BLOCK_M, M);
        
        for (int bn = 0; bn < N; bn += L2_BLOCK_N) {
            const int max_j_l2 = std::min(bn + L2_BLOCK_N, N);
            
            for (int bk = 0; bk < K; bk += L2_BLOCK_K) {
                const int max_k_l2 = std::min(bk + L2_BLOCK_K, K);

                // L1 cache blocking
                for (int i = bm; i < max_i_l2; i += L1_BLOCK_M) {
                    const int max_i_l1 = std::min(i + L1_BLOCK_M, max_i_l2);
                    
                    for (int j = bn; j < max_j_l2; j += L1_BLOCK_N) {
                        const int max_j_l1 = std::min(j + L1_BLOCK_N, max_j_l2);

                        // Register blocking
                        for (int ii = i; ii < max_i_l1; ii += REG_BLOCK_M) {
                            const int max_i_reg = std::min(ii + REG_BLOCK_M, max_i_l1);
                            
                            for (int jj = j; jj < max_j_l1; jj += REG_BLOCK_N) {
                                const int max_j_reg = std::min(jj + REG_BLOCK_N, max_j_l1);

                                // Register accumulators
                                SimpleMatrix::value_type acc[REG_BLOCK_M][REG_BLOCK_N] = {};
                                
                                // Load existing values if not starting K block
                                if (bk > 0) {
                                    for (int ri = 0; ri < max_i_reg - ii; ++ri) {
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
                                        // Cache rhs values
                                        SimpleMatrix::value_type rhs_cache[REG_BLOCK_N];
                                        #pragma unroll
                                        for (int rj = 0; rj < max_j_reg - jj; ++rj) {
                                            rhs_cache[rj] = rhs(kk, jj + rj);
                                        }

                                        // Compute partial products
                                        #pragma unroll
                                        for (int ri = 0; ri < max_i_reg - ii; ++ri) {
                                            const SimpleMatrix::value_type lhs_val = lhs(ii + ri, kk);
                                            #pragma unroll
                                            for (int rj = 0; rj < max_j_reg - jj; ++rj) {
                                                acc[ri][rj] += lhs_val * rhs_cache[rj];
                                            }
                                        }
                                    }
                                }

                                // Store results
                                for (int ri = 0; ri < max_i_reg - ii; ++ri) {
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

    return res;
}
