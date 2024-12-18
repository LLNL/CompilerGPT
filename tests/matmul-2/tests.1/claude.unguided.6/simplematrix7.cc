
#include <stdexcept>
#include "simplematrix.h"

namespace {
    // Highly optimized 4x4 multiplication using accumulator pattern
    void multiply_4x4(const SimpleMatrix& lhs, const SimpleMatrix& rhs, SimpleMatrix& res) {
        using value_type = SimpleMatrix::value_type;
        value_type acc[4][4] = {{}}; // Zero-initialized accumulators
        
        for (int k = 0; k < 4; ++k) {
            for (int i = 0; i < 4; ++i) {
                const value_type lhs_ik = lhs(i,k);
                acc[i][0] += lhs_ik * rhs(k,0);
                acc[i][1] += lhs_ik * rhs(k,1);
                acc[i][2] += lhs_ik * rhs(k,2);
                acc[i][3] += lhs_ik * rhs(k,3);
            }
        }
        
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                res(i,j) = acc[i][j];
            }
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

    // Specialized fast path for 4x4
    if (M == 4 && N == 4 && K == 4) {
        multiply_4x4(lhs, rhs, res);
        return res;
    }

    // Fast path for small matrices
    if (M <= 16 && N <= 16) {
        constexpr int UNROLL = 4;
        value_type acc[16][16] = {{}};  // Zero-initialized accumulators

        for (int k = 0; k < K; ++k) {
            for (int i = 0; i < M; ++i) {
                const value_type lhs_ik = lhs(i,k);
                for (int j = 0; j < N; ++j) {
                    acc[i][j] += lhs_ik * rhs(k,j);
                }
            }
        }

        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j) {
                res(i,j) = acc[i][j];
            }
        }
        return res;
    }

    // Parameters tuned for common L1/L2/L3 cache sizes
    constexpr int L1_BLOCK = 64;   // L1 cache block
    constexpr int MINI_BLOCK = 8;  // Mini block for register optimization
    constexpr int INNER_KERNEL = 4; // Inner kernel size

    // Initialize result to zero
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            res(i,j) = value_type{0};
        }
    }

    // Main computation with optimized blocking
    for (int i0 = 0; i0 < M; i0 += L1_BLOCK) {
        const int imax = std::min(i0 + L1_BLOCK, M);
        
        for (int k0 = 0; k0 < K; k0 += L1_BLOCK) {
            const int kmax = std::min(k0 + L1_BLOCK, K);
            
            for (int j0 = 0; j0 < N; j0 += L1_BLOCK) {
                const int jmax = std::min(j0 + L1_BLOCK, N);
                
                // Mini-blocked multiplication
                for (int i = i0; i < imax; i += MINI_BLOCK) {
                    const int iimax = std::min(i + MINI_BLOCK, imax);
                    
                    for (int k = k0; k < kmax; k += INNER_KERNEL) {
                        const int kkmax = std::min(k + INNER_KERNEL, kmax);
                        
                        // Cache lhs values for current mini-block
                        value_type lhs_cache[MINI_BLOCK][INNER_KERNEL];
                        for (int ii = i; ii < iimax; ++ii) {
                            for (int kk = k; kk < kkmax; ++kk) {
                                lhs_cache[ii-i][kk-k] = lhs(ii,kk);
                            }
                        }
                        
                        for (int j = j0; j < jmax; j += MINI_BLOCK) {
                            const int jjmax = std::min(j + MINI_BLOCK, jmax);
                            
                            // Local accumulator for better register usage
                            value_type local_acc[MINI_BLOCK][MINI_BLOCK] = {{}};
                            
                            // Inner kernel computation
                            for (int kk = k; kk < kkmax; ++kk) {
                                value_type rhs_cache[MINI_BLOCK];
                                for (int jj = j; jj < jjmax; ++jj) {
                                    rhs_cache[jj-j] = rhs(kk,jj);
                                }
                                
                                for (int ii = i; ii < iimax; ++ii) {
                                    const value_type lhs_val = lhs_cache[ii-i][kk-k];
                                    for (int jj = j; jj < jjmax; ++jj) {
                                        local_acc[ii-i][jj-j] += lhs_val * rhs_cache[jj-j];
                                    }
                                }
                            }
                            
                            // Update result matrix
                            for (int ii = i; ii < iimax; ++ii) {
                                for (int jj = j; jj < jjmax; ++jj) {
                                    res(ii,jj) += local_acc[ii-i][jj-j];
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
