
#include <stdexcept>
#include "simplematrix.h"

namespace {
    // Specialized multiplication for tiny matrices (2x2)
    void multiply_2x2(const SimpleMatrix& lhs, const SimpleMatrix& rhs, SimpleMatrix& res) {
        using value_type = SimpleMatrix::value_type;
        res(0,0) = lhs(0,0) * rhs(0,0) + lhs(0,1) * rhs(1,0);
        res(0,1) = lhs(0,0) * rhs(0,1) + lhs(0,1) * rhs(1,1);
        res(1,0) = lhs(1,0) * rhs(0,0) + lhs(1,1) * rhs(1,0);
        res(1,1) = lhs(1,0) * rhs(0,1) + lhs(1,1) * rhs(1,1);
    }

    // Optimized 4x4 multiplication
    void multiply_4x4(const SimpleMatrix& lhs, const SimpleMatrix& rhs, SimpleMatrix& res) {
        using value_type = SimpleMatrix::value_type;
        for (int i = 0; i < 4; ++i) {
            value_type sum0{0}, sum1{0}, sum2{0}, sum3{0};
            for (int k = 0; k < 4; ++k) {
                const value_type lhs_ik = lhs(i,k);
                sum0 += lhs_ik * rhs(k,0);
                sum1 += lhs_ik * rhs(k,1);
                sum2 += lhs_ik * rhs(k,2);
                sum3 += lhs_ik * rhs(k,3);
            }
            res(i,0) = sum0;
            res(i,1) = sum1;
            res(i,2) = sum2;
            res(i,3) = sum3;
        }
    }
}

SimpleMatrix operator*(const SimpleMatrix& lhs, const SimpleMatrix& rhs) 
{
    if (lhs.columns() != rhs.rows())
        throw std::runtime_error{"lhs.columns() != rhs.rows()"};

    const int M = lhs.rows();
    const int N = rhs.columns();
    const int K = lhs.columns();
    
    SimpleMatrix res{M, N};
    using value_type = SimpleMatrix::value_type;

    // Specialized paths for small matrices
    if (M == 2 && N == 2 && K == 2) {
        multiply_2x2(lhs, rhs, res);
        return res;
    }
    if (M == 4 && N == 4 && K == 4) {
        multiply_4x4(lhs, rhs, res);
        return res;
    }

    // Fast path for very small matrices
    if (M <= 8 && N <= 8) {
        constexpr int UNROLL = 4;
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                value_type sum[UNROLL] = {value_type{0}};
                int k = 0;
                
                // Unrolled accumulation
                for (; k + (UNROLL-1) < K; k += UNROLL) {
                    for (int u = 0; u < UNROLL; ++u) {
                        sum[u] += lhs(i, k+u) * rhs(k+u, j);
                    }
                }
                
                // Combine unrolled sums
                value_type final_sum = sum[0] + sum[1] + sum[2] + sum[3];
                for (; k < K; ++k) {
                    final_sum += lhs(i, k) * rhs(k, j);
                }
                res(i, j) = final_sum;
            }
        }
        return res;
    }

    // Initialize result
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i, j) = value_type{0};
        }
    }

    // Cache-optimized blocking parameters
    constexpr int L2_BLOCK = 128;  // L2 cache block
    constexpr int L1_BLOCK = 32;   // L1 cache block
    constexpr int REG_BLOCK = 4;   // Register block

    // Main computation with three-level blocking
    for (int i2 = 0; i2 < M; i2 += L2_BLOCK) {
        const int imax2 = std::min(i2 + L2_BLOCK, M);
        
        for (int k2 = 0; k2 < K; k2 += L2_BLOCK) {
            const int kmax2 = std::min(k2 + L2_BLOCK, K);
            
            for (int j2 = 0; j2 < N; j2 += L2_BLOCK) {
                const int jmax2 = std::min(j2 + L2_BLOCK, N);
                
                // L1 cache blocking
                for (int i1 = i2; i1 < imax2; i1 += L1_BLOCK) {
                    const int imax1 = std::min(i1 + L1_BLOCK, imax2);
                    
                    for (int k1 = k2; k1 < kmax2; k1 += L1_BLOCK) {
                        const int kmax1 = std::min(k1 + L1_BLOCK, kmax2);
                        
                        // Register blocking
                        for (int i = i1; i < imax1; i += REG_BLOCK) {
                            const int imax = std::min(i + REG_BLOCK, imax1);
                            
                            value_type lhs_cache[REG_BLOCK][L1_BLOCK];
                            
                            // Prefetch lhs block
                            for (int ii = i; ii < imax; ++ii) {
                                for (int k = k1; k < kmax1; ++k) {
                                    lhs_cache[ii-i][k-k1] = lhs(ii, k);
                                }
                            }
                            
                            for (int j = j2; j < jmax2; j += REG_BLOCK) {
                                const int jmax = std::min(j + REG_BLOCK, jmax2);
                                
                                // Initialize accumulators
                                value_type local_sum[REG_BLOCK][REG_BLOCK] = {};
                                
                                // Core computation with cached values
                                for (int k = k1; k < kmax1; ++k) {
                                    value_type rhs_cache[REG_BLOCK];
                                    for (int jj = j; jj < jmax; ++jj) {
                                        rhs_cache[jj-j] = rhs(k, jj);
                                    }
                                    
                                    for (int ii = i; ii < imax; ++ii) {
                                        const value_type lhs_val = lhs_cache[ii-i][k-k1];
                                        for (int jj = j; jj < jmax; ++jj) {
                                            local_sum[ii-i][jj-j] += lhs_val * rhs_cache[jj-j];
                                        }
                                    }
                                }
                                
                                // Update result matrix
                                for (int ii = i; ii < imax; ++ii) {
                                    for (int jj = j; jj < jmax; ++jj) {
                                        res(ii, jj) += local_sum[ii-i][jj-j];
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
