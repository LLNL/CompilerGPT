
#include <stdexcept>
#include "simplematrix.h"

namespace {
    // Helper function for small matrix multiplication with 4x4 blocking
    void multiply_small(const SimpleMatrix& lhs, const SimpleMatrix& rhs, 
                       SimpleMatrix& res, int M, int N, int K) {
        using value_type = SimpleMatrix::value_type;
        
        // Process 4x4 blocks
        for (int i = 0; i < M; i += 4) {
            for (int j = 0; j < N; j += 4) {
                value_type sum[4][4] = {{0}}; // Initialize all elements to 0
                
                for (int k = 0; k < K; ++k) {
                    const value_type lhs_ik0 = (i < M) ? lhs(i, k) : 0;
                    const value_type lhs_ik1 = (i + 1 < M) ? lhs(i + 1, k) : 0;
                    const value_type lhs_ik2 = (i + 2 < M) ? lhs(i + 2, k) : 0;
                    const value_type lhs_ik3 = (i + 3 < M) ? lhs(i + 3, k) : 0;
                    
                    const value_type rhs_kj0 = (j < N) ? rhs(k, j) : 0;
                    const value_type rhs_kj1 = (j + 1 < N) ? rhs(k, j + 1) : 0;
                    const value_type rhs_kj2 = (j + 2 < N) ? rhs(k, j + 2) : 0;
                    const value_type rhs_kj3 = (j + 3 < N) ? rhs(k, j + 3) : 0;
                    
                    // Compute 4x4 block
                    sum[0][0] += lhs_ik0 * rhs_kj0;
                    sum[0][1] += lhs_ik0 * rhs_kj1;
                    sum[0][2] += lhs_ik0 * rhs_kj2;
                    sum[0][3] += lhs_ik0 * rhs_kj3;
                    
                    sum[1][0] += lhs_ik1 * rhs_kj0;
                    sum[1][1] += lhs_ik1 * rhs_kj1;
                    sum[1][2] += lhs_ik1 * rhs_kj2;
                    sum[1][3] += lhs_ik1 * rhs_kj3;
                    
                    sum[2][0] += lhs_ik2 * rhs_kj0;
                    sum[2][1] += lhs_ik2 * rhs_kj1;
                    sum[2][2] += lhs_ik2 * rhs_kj2;
                    sum[2][3] += lhs_ik2 * rhs_kj3;
                    
                    sum[3][0] += lhs_ik3 * rhs_kj0;
                    sum[3][1] += lhs_ik3 * rhs_kj1;
                    sum[3][2] += lhs_ik3 * rhs_kj2;
                    sum[3][3] += lhs_ik3 * rhs_kj3;
                }
                
                // Store results
                for (int ii = 0; ii < 4 && i + ii < M; ++ii) {
                    for (int jj = 0; jj < 4 && j + jj < N; ++jj) {
                        res(i + ii, j + jj) = sum[ii][jj];
                    }
                }
            }
        }
    }
}

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) {
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};

    // Small matrix optimization
    if (M <= 64 && N <= 64 && K <= 64) {
        multiply_small(lhs, rhs, res, M, N, K);
        return res;
    }

    using value_type = SimpleMatrix::value_type;
    
    // Cache blocking parameters
    constexpr int L2_BLOCK = 128;  // L2 cache block
    constexpr int L1_BLOCK = 32;   // L1 cache block
    constexpr int REG_BLOCK = 4;   // Register block

    // Initialize result matrix
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i,j) = 0;
        }
    }

    // Three-level blocking (L2 cache -> L1 cache -> registers)
    for (int i2 = 0; i2 < M; i2 += L2_BLOCK) {
        const int i2_end = std::min(i2 + L2_BLOCK, M);
        
        for (int j2 = 0; j2 < N; j2 += L2_BLOCK) {
            const int j2_end = std::min(j2 + L2_BLOCK, N);
            
            for (int k2 = 0; k2 < K; k2 += L2_BLOCK) {
                const int k2_end = std::min(k2 + L2_BLOCK, K);

                // L1 cache blocking
                for (int i1 = i2; i1 < i2_end; i1 += L1_BLOCK) {
                    const int i1_end = std::min(i1 + L1_BLOCK, i2_end);
                    
                    for (int j1 = j2; j1 < j2_end; j1 += L1_BLOCK) {
                        const int j1_end = std::min(j1 + L1_BLOCK, j2_end);
                        
                        for (int k1 = k2; k1 < k2_end; k1 += L1_BLOCK) {
                            const int k1_end = std::min(k1 + L1_BLOCK, k2_end);

                            // Register blocking
                            for (int i = i1; i < i1_end; i += REG_BLOCK) {
                                const int i_end = std::min(i + REG_BLOCK, i1_end);
                                
                                for (int k = k1; k < k1_end; ++k) {
                                    for (int j = j1; j < j1_end; j += REG_BLOCK) {
                                        const int j_end = std::min(j + REG_BLOCK, j1_end);
                                        
                                        value_type reg[REG_BLOCK] = {0};
                                        
                                        // Load and compute register block
                                        for (int ii = i; ii < i_end; ++ii) {
                                            const value_type lhs_val = lhs(ii, k);
                                            for (int jj = j; jj < j_end; ++jj) {
                                                reg[jj-j] += lhs_val * rhs(k, jj);
                                            }
                                        }
                                        
                                        // Store register block
                                        for (int ii = i; ii < i_end; ++ii) {
                                            for (int jj = j; jj < j_end; ++jj) {
                                                res(ii, jj) += reg[jj-j];
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
