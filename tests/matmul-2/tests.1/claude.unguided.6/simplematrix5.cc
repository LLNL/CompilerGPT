
#include <stdexcept>
#include "simplematrix.h"

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) 
{
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};
    using value_type = SimpleMatrix::value_type;

    // Fast path for very small matrices (4x4 or smaller)
    if (M <= 4 && N <= 4 && K <= 4) {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                value_type sum{0};
                for (int k = 0; k < K; ++k) {
                    sum += lhs(i, k) * rhs(k, j);
                }
                res(i, j) = sum;
            }
        }
        return res;
    }

    // Medium path for small matrices (8x8 or smaller)
    if (M <= 8 && N <= 8) {
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                value_type sum0{0}, sum1{0}, sum2{0}, sum3{0};
                int k = 0;
                
                // Process 4 elements at once
                for (; k + 3 < K; k += 4) {
                    sum0 += lhs(i, k) * rhs(k, j);
                    sum1 += lhs(i, k+1) * rhs(k+1, j);
                    sum2 += lhs(i, k+2) * rhs(k+2, j);
                    sum3 += lhs(i, k+3) * rhs(k+3, j);
                }
                
                // Handle remaining elements
                value_type sum = sum0 + sum1 + sum2 + sum3;
                for (; k < K; ++k) {
                    sum += lhs(i, k) * rhs(k, j);
                }
                res(i, j) = sum;
            }
        }
        return res;
    }

    // Initialize result matrix
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i, j) = value_type{0};
        }
    }

    constexpr int BLOCK_L2 = 64;  // L2 cache block size
    constexpr int BLOCK_L1 = 32;  // L1 cache block size
    constexpr int MINI_BLOCK = 4; // Register block size

    // Three-level blocking strategy
    for (int i2 = 0; i2 < M; i2 += BLOCK_L2) {
        const int imax2 = std::min(i2 + BLOCK_L2, M);
        
        for (int k2 = 0; k2 < K; k2 += BLOCK_L2) {
            const int kmax2 = std::min(k2 + BLOCK_L2, K);
            
            for (int j2 = 0; j2 < N; j2 += BLOCK_L2) {
                const int jmax2 = std::min(j2 + BLOCK_L2, N);
                
                // L1 cache blocking
                for (int i1 = i2; i1 < imax2; i1 += BLOCK_L1) {
                    const int imax1 = std::min(i1 + BLOCK_L1, imax2);
                    
                    for (int k1 = k2; k1 < kmax2; k1 += BLOCK_L1) {
                        const int kmax1 = std::min(k1 + BLOCK_L1, kmax2);
                        
                        // Register blocking
                        for (int i = i1; i < imax1; i += MINI_BLOCK) {
                            const int imax = std::min(i + MINI_BLOCK, imax1);
                            
                            for (int k = k1; k < kmax1; ++k) {
                                // Cache lhs values
                                value_type lhs_cache[MINI_BLOCK];
                                for (int ii = i; ii < imax; ++ii) {
                                    lhs_cache[ii-i] = lhs(ii, k);
                                }
                                
                                for (int j = j2; j < jmax2; j += MINI_BLOCK) {
                                    const int jmax = std::min(j + MINI_BLOCK, jmax2);
                                    
                                    // Cache rhs values
                                    value_type rhs_cache[MINI_BLOCK];
                                    for (int jj = j; jj < jmax; ++jj) {
                                        rhs_cache[jj-j] = rhs(k, jj);
                                    }
                                    
                                    // Core multiplication with cached values
                                    for (int ii = i; ii < imax; ++ii) {
                                        const value_type lhs_val = lhs_cache[ii-i];
                                        for (int jj = j; jj < jmax; ++jj) {
                                            res(ii, jj) += lhs_val * rhs_cache[jj-j];
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
